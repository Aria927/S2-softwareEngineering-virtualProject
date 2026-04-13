#include "password_manager_backend/VaultService.hpp"

#include <random>

namespace password_manager_backend {

VaultService::VaultService(
    VaultRepository& vaultRepository,
    AuthService& authService,
    CryptoService& cryptoService,
    SearchService& searchService)
    : vaultRepository_(vaultRepository),
      authService_(authService),
      cryptoService_(cryptoService),
      searchService_(searchService) {}

Result<VaultEntryView> VaultService::createVaultEntry(const VaultEntryInput& input) {
    const auto validationResult = validateEntryInput(input);
    if (!validationResult.success) {
        return {false, validationResult.message, std::nullopt};
    }

    const auto& session = authService_.getSessionState();
    if (!session.isAuthenticated || !session.isVaultUnlocked || !session.userId.has_value() ||
        !session.activeEncryptionKey.has_value()) {
        return {false, "You must unlock the vault before managing entries.", std::nullopt};
    }

    // Plaintext is accepted only at the boundary, then encrypted before the entry is stored.
    const EncryptedBlob encryptedPassword =
        cryptoService_.encrypt(input.password, *session.activeEncryptionKey);

    const VaultEntry entry{
        .id = createIdentifier(),
        .userId = *session.userId,
        .title = input.title,
        .site = input.site,
        .username = input.username,
        .encryptedPassword = encryptedPassword.cipherText,
        .encryptionSalt = encryptedPassword.salt,
        .encryptionIv = encryptedPassword.iv,
        .notes = input.notes,
        .tags = input.tags,
    };

    vaultRepository_.save(entry);
    return toView(entry);
}

Result<std::vector<VaultEntryView>> VaultService::getVaultEntries() const {
    return listEntries();
}

Result<VaultEntryView> VaultService::updateVaultEntry(const std::string& entryId, const VaultEntryInput& input) {
    const auto validationResult = validateEntryInput(input);
    if (!validationResult.success) {
        return {false, validationResult.message, std::nullopt};
    }

    const auto& session = authService_.getSessionState();
    if (!session.isAuthenticated || !session.isVaultUnlocked || !session.userId.has_value() ||
        !session.activeEncryptionKey.has_value()) {
        return {false, "You must unlock the vault before managing entries.", std::nullopt};
    }

    const auto existingEntry = vaultRepository_.findById(entryId);
    if (!existingEntry.has_value() || existingEntry->userId != *session.userId) {
        return {false, "Vault entry not found.", std::nullopt};
    }

    // Updating re-encrypts the password so the repository never receives plaintext secrets.
    const EncryptedBlob encryptedPassword =
        cryptoService_.encrypt(input.password, *session.activeEncryptionKey);

    VaultEntry updatedEntry = *existingEntry;
    updatedEntry.title = input.title;
    updatedEntry.site = input.site;
    updatedEntry.username = input.username;
    updatedEntry.encryptedPassword = encryptedPassword.cipherText;
    updatedEntry.encryptionSalt = encryptedPassword.salt;
    updatedEntry.encryptionIv = encryptedPassword.iv;
    updatedEntry.notes = input.notes;
    updatedEntry.tags = input.tags;

    vaultRepository_.update(updatedEntry);
    return toView(updatedEntry);
}

OperationResult VaultService::deleteVaultEntry(const std::string& entryId) {
    const auto& session = authService_.getSessionState();
    if (!session.isAuthenticated || !session.isVaultUnlocked || !session.userId.has_value()) {
        return {false, "You must unlock the vault before managing entries."};
    }

    const bool wasDeleted = vaultRepository_.remove(entryId, *session.userId);
    if (!wasDeleted) {
        return {false, "Vault entry not found."};
    }

    return {true, "Vault entry deleted."};
}

Result<std::vector<VaultEntryView>> VaultService::searchVaultEntries(const SearchQuery& query) const {
    const auto entriesResult = listEntries();
    if (!entriesResult.success || !entriesResult.value.has_value()) {
        return entriesResult;
    }

    return {
        true,
        "Search complete.",
        searchService_.search(*entriesResult.value, query),
    };
}

std::string VaultService::createIdentifier() {
    // Entry IDs are generated here to keep repository contracts minimal and storage-agnostic.
    static constexpr char alphabet[] = "0123456789abcdef";
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<std::size_t> distribution(0, sizeof(alphabet) - 2);

    std::string identifier(24, '0');
    for (char& character : identifier) {
        character = alphabet[distribution(generator)];
    }
    return identifier;
}

OperationResult VaultService::validateEntryInput(const VaultEntryInput& input) {
    // Validation is intentionally lightweight and focused on fields the backend truly depends on.
    if (input.title.empty()) {
        return {false, "Entry title is required."};
    }

    if (input.username.empty()) {
        return {false, "Entry username is required."};
    }

    if (input.password.empty()) {
        return {false, "Entry password is required."};
    }

    return {true, "Entry is valid."};
}

Result<VaultEntryView> VaultService::toView(const VaultEntry& entry) const {
    const auto& session = authService_.getSessionState();
    if (!session.activeEncryptionKey.has_value()) {
        return {false, "The vault is locked.", std::nullopt};
    }

    // Passwords are decrypted only when converting a stored entry into a GUI-facing view model.
    const EncryptedBlob encryptedPassword{
        entry.encryptedPassword,
        entry.encryptionSalt,
        entry.encryptionIv,
    };

    return {
        true,
        "Vault entry loaded.",
        VaultEntryView{
            .id = entry.id,
            .title = entry.title,
            .site = entry.site,
            .username = entry.username,
            .password = cryptoService_.decrypt(encryptedPassword, *session.activeEncryptionKey),
            .notes = entry.notes,
            .tags = entry.tags,
        },
    };
}

Result<std::vector<VaultEntryView>> VaultService::listEntries() const {
    const auto& session = authService_.getSessionState();
    if (!session.isAuthenticated || !session.isVaultUnlocked || !session.userId.has_value()) {
        return {false, "You must unlock the vault before viewing entries.", std::nullopt};
    }

    // Entries remain encrypted in the repository; this method performs the unlock-time transformation for the GUI.
    std::vector<VaultEntryView> decryptedEntries;
    for (const auto& entry : vaultRepository_.findByUserId(*session.userId)) {
        auto entryView = toView(entry);
        if (!entryView.success || !entryView.value.has_value()) {
            return {false, entryView.message, std::nullopt};
        }
        decryptedEntries.push_back(*entryView.value);
    }

    return {true, "Vault entries loaded.", decryptedEntries};
}

}  // namespace password_manager_backend
