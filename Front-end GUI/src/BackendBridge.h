#pragma once

#include "../../include/password_manager_backend/AuditService.hpp"
#include "../../include/password_manager_backend/AuthService.hpp"
#include "../../include/password_manager_backend/CryptoService.hpp"
#include "../../include/password_manager_backend/MfaService.hpp"
#include "../../include/password_manager_backend/MySqlRepositories.hpp"
#include "../../include/password_manager_backend/Repositories.hpp"
#include "../../include/password_manager_backend/SearchService.hpp"
#include "../../include/password_manager_backend/VaultService.hpp"
#include "PasswordEntry.h"

#include <memory>

class GuiUserRepository final : public password_manager_backend::UserRepository {
public:
    GuiUserRepository();

    void useImplementation(std::unique_ptr<password_manager_backend::UserRepository> implementation);

    std::optional<password_manager_backend::UserAccount> findByUsername(const std::string& username) const override;
    std::optional<password_manager_backend::UserAccount> findByEmail(const std::string& email) const override;
    std::optional<password_manager_backend::UserAccount> findById(const std::string& id) const override;
    std::vector<password_manager_backend::UserAccount> listAll() const override;
    void save(const password_manager_backend::UserAccount& user) override;
    void update(const password_manager_backend::UserAccount& user) override;

private:
    std::unique_ptr<password_manager_backend::UserRepository> implementation_;
};

class GuiVaultRepository final : public password_manager_backend::VaultRepository {
public:
    GuiVaultRepository();

    void useImplementation(std::unique_ptr<password_manager_backend::VaultRepository> implementation);

    void save(const password_manager_backend::VaultEntry& entry) override;
    std::optional<password_manager_backend::VaultEntry> findById(const std::string& entryId) const override;
    std::vector<password_manager_backend::VaultEntry> findByUserId(const std::string& userId) const override;
    void update(const password_manager_backend::VaultEntry& entry) override;
    bool remove(const std::string& entryId, const std::string& userId) override;

private:
    std::unique_ptr<password_manager_backend::VaultRepository> implementation_;
};

class GuiAuditRepository final : public password_manager_backend::AuditRepository {
public:
    GuiAuditRepository();

    void useImplementation(std::unique_ptr<password_manager_backend::AuditRepository> implementation);

    void save(const password_manager_backend::AuditEvent& event) override;
    std::vector<password_manager_backend::AuditEvent> listAll() const override;
    std::vector<password_manager_backend::AuditEvent> listByUserId(const std::string& userId) const override;

private:
    std::unique_ptr<password_manager_backend::AuditRepository> implementation_;
};

// This bridge keeps the GUI code simple by translating button-driven UI actions into backend service calls.
class BackendBridge {
public:
    BackendBridge();

    bool registerAccount(const std::string& email, const std::string& password, const std::string& confirmation, std::string& message);
    bool login(const std::string& email, const std::string& password, std::string& message);
    bool logout(std::string& message);
    bool addEntry(
        const std::string& appName,
        const std::string& username,
        const std::string& password,
        const std::string& notes,
        std::string& message);
    bool updateEntry(
        const std::string& entryId,
        const std::string& appName,
        const std::string& username,
        const std::string& password,
        const std::string& notes,
        std::string& message);
    bool deleteEntry(const std::string& entryId, std::string& message);
    std::vector<PasswordEntry> getAllEntries(std::string& message);
    std::vector<PasswordEntry> searchEntries(const std::string& query, std::string& message);

private:
    std::vector<PasswordEntry> mapEntries(const std::vector<password_manager_backend::VaultEntryView>& entries) const;

    GuiUserRepository userRepository_;
    GuiVaultRepository vaultRepository_;
    GuiAuditRepository auditRepository_;
    std::shared_ptr<password_manager_backend::MySqlDatabase> mySqlDatabase_;
    password_manager_backend::CryptoService cryptoService_;
    password_manager_backend::MfaService mfaService_;
    password_manager_backend::AuditService auditService_;
    password_manager_backend::SearchService searchService_;
    password_manager_backend::AuthService authService_;
    password_manager_backend::VaultService vaultService_;
};
