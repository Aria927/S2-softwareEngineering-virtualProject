#include "BackendBridge.h"

namespace {

constexpr const char* kGuiRecoveryKey = "gui-integration-recovery-key";

}

BackendBridge::BackendBridge()
    : mfaService_(cryptoService_),
      auditService_(auditRepository_),
      authService_(userRepository_, cryptoService_, mfaService_, auditService_, kGuiRecoveryKey),
      vaultService_(vaultRepository_, authService_, cryptoService_, searchService_, auditService_) {}

bool BackendBridge::registerAccount(
    const std::string& email,
    const std::string& password,
    const std::string& confirmation,
    std::string& message) {
    if (email.empty() || password.empty()) {
        message = "Email and password are required.";
        return false;
    }

    if (password != confirmation) {
        message = "Passwords do not match.";
        return false;
    }

    // The current GUI does not yet have separate username and master-password fields, so this integration uses the
    // email as the username and the same password for both account login and vault unlock until the GUI expands.
    const auto result = authService_.registerUser({
        .username = email,
        .email = email,
        .password = password,
        .masterPassword = password,
        .isAdmin = false,
    });

    message = result.message;
    return result.success;
}

bool BackendBridge::login(const std::string& email, const std::string& password, std::string& message) {
    const auto loginResult = authService_.login({
        .username = email,
        .password = password,
    });

    if (!loginResult.success) {
        message = loginResult.message;
        return false;
    }

    const auto unlockResult = authService_.verifyMasterPassword(password);
    message = unlockResult.message;
    return unlockResult.success;
}

bool BackendBridge::logout(std::string& message) {
    const auto result = authService_.logout();
    message = result.message;
    return result.success;
}

bool BackendBridge::addEntry(
    const std::string& appName,
    const std::string& username,
    const std::string& password,
    std::string& message) {
    const auto result = vaultService_.createVaultEntry({
        .title = appName,
        .site = appName,
        .username = username,
        .password = password,
        .notes = "",
        .tags = {},
    });

    message = result.message;
    return result.success;
}

bool BackendBridge::updateEntry(
    const std::string& entryId,
    const std::string& appName,
    const std::string& username,
    const std::string& password,
    std::string& message) {
    const auto result = vaultService_.updateVaultEntry(entryId, {
        .title = appName,
        .site = appName,
        .username = username,
        .password = password,
        .notes = "",
        .tags = {},
    });

    message = result.message;
    return result.success;
}

bool BackendBridge::deleteEntry(const std::string& entryId, std::string& message) {
    const auto result = vaultService_.deleteVaultEntry(entryId);
    message = result.message;
    return result.success;
}

std::vector<PasswordEntry> BackendBridge::getAllEntries(std::string& message) {
    const auto result = vaultService_.getVaultEntries();
    message = result.message;

    if (!result.success || !result.value.has_value()) {
        return {};
    }

    return mapEntries(*result.value);
}

std::vector<PasswordEntry> BackendBridge::searchEntries(const std::string& query, std::string& message) {
    const auto result = vaultService_.searchVaultEntries({.text = query});
    message = result.message;

    if (!result.success || !result.value.has_value()) {
        return {};
    }

    return mapEntries(*result.value);
}

std::vector<PasswordEntry> BackendBridge::mapEntries(
    const std::vector<password_manager_backend::VaultEntryView>& entries) const {
    std::vector<PasswordEntry> mappedEntries;
    mappedEntries.reserve(entries.size());

    for (const auto& entry : entries) {
        mappedEntries.push_back({
            .id = entry.id,
            .appName = entry.title,
            .username = entry.username,
            .password = entry.password,
        });
    }

    return mappedEntries;
}
