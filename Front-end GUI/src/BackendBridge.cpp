#include "BackendBridge.h"

#include <stdexcept>

namespace {

constexpr const char* kGuiRecoveryKey = "gui-integration-recovery-key";

}

GuiUserRepository::GuiUserRepository()
    : implementation_(std::make_unique<password_manager_backend::InMemoryUserRepository>()) {}

void GuiUserRepository::useImplementation(std::unique_ptr<password_manager_backend::UserRepository> implementation) {
    implementation_ = std::move(implementation);
}

std::optional<password_manager_backend::UserAccount> GuiUserRepository::findByUsername(const std::string& username) const {
    return implementation_->findByUsername(username);
}

std::optional<password_manager_backend::UserAccount> GuiUserRepository::findByEmail(const std::string& email) const {
    return implementation_->findByEmail(email);
}

std::optional<password_manager_backend::UserAccount> GuiUserRepository::findById(const std::string& id) const {
    return implementation_->findById(id);
}

std::vector<password_manager_backend::UserAccount> GuiUserRepository::listAll() const {
    return implementation_->listAll();
}

void GuiUserRepository::save(const password_manager_backend::UserAccount& user) {
    implementation_->save(user);
}

void GuiUserRepository::update(const password_manager_backend::UserAccount& user) {
    implementation_->update(user);
}

GuiVaultRepository::GuiVaultRepository()
    : implementation_(std::make_unique<password_manager_backend::InMemoryVaultRepository>()) {}

void GuiVaultRepository::useImplementation(std::unique_ptr<password_manager_backend::VaultRepository> implementation) {
    implementation_ = std::move(implementation);
}

void GuiVaultRepository::save(const password_manager_backend::VaultEntry& entry) {
    implementation_->save(entry);
}

std::optional<password_manager_backend::VaultEntry> GuiVaultRepository::findById(const std::string& entryId) const {
    return implementation_->findById(entryId);
}

std::vector<password_manager_backend::VaultEntry> GuiVaultRepository::findByUserId(const std::string& userId) const {
    return implementation_->findByUserId(userId);
}

void GuiVaultRepository::update(const password_manager_backend::VaultEntry& entry) {
    implementation_->update(entry);
}

bool GuiVaultRepository::remove(const std::string& entryId, const std::string& userId) {
    return implementation_->remove(entryId, userId);
}

GuiAuditRepository::GuiAuditRepository()
    : implementation_(std::make_unique<password_manager_backend::InMemoryAuditRepository>()) {}

void GuiAuditRepository::useImplementation(std::unique_ptr<password_manager_backend::AuditRepository> implementation) {
    implementation_ = std::move(implementation);
}

void GuiAuditRepository::save(const password_manager_backend::AuditEvent& event) {
    implementation_->save(event);
}

std::vector<password_manager_backend::AuditEvent> GuiAuditRepository::listAll() const {
    return implementation_->listAll();
}

std::vector<password_manager_backend::AuditEvent> GuiAuditRepository::listByUserId(const std::string& userId) const {
    return implementation_->listByUserId(userId);
}

BackendBridge::BackendBridge()
    : mfaService_(cryptoService_),
      auditService_(auditRepository_),
      authService_(userRepository_, cryptoService_, mfaService_, auditService_, kGuiRecoveryKey),
      vaultService_(vaultRepository_, authService_, cryptoService_, searchService_, auditService_) {
    if (!password_manager_backend::MySqlConnectionConfig::isEnabledFromEnvironment()) {
        return;
    }

    try {
        auto configuration = password_manager_backend::MySqlConnectionConfig::fromEnvironment();
        mySqlDatabase_ = std::make_shared<password_manager_backend::MySqlDatabase>(std::move(configuration));

        userRepository_.useImplementation(
            std::make_unique<password_manager_backend::MySqlUserRepository>(mySqlDatabase_));
        vaultRepository_.useImplementation(
            std::make_unique<password_manager_backend::MySqlVaultRepository>(mySqlDatabase_));
        auditRepository_.useImplementation(
            std::make_unique<password_manager_backend::MySqlAuditRepository>(mySqlDatabase_));
    } catch (const std::exception&) {
        // The GUI keeps its in-memory fallback so the app still runs on machines without working MySQL credentials.
        mySqlDatabase_.reset();
    }
}

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
