#pragma once

#include "../../include/password_manager_backend/AuditService.hpp"
#include "../../include/password_manager_backend/AuthService.hpp"
#include "../../include/password_manager_backend/CryptoService.hpp"
#include "../../include/password_manager_backend/MfaService.hpp"
#include "../../include/password_manager_backend/Repositories.hpp"
#include "../../include/password_manager_backend/SearchService.hpp"
#include "../../include/password_manager_backend/VaultService.hpp"
#include "PasswordEntry.h"

// This bridge keeps the GUI code simple by translating button-driven UI actions into backend service calls.
class BackendBridge {
public:
    BackendBridge();

    bool registerAccount(const std::string& email, const std::string& password, const std::string& confirmation, std::string& message);
    bool login(const std::string& email, const std::string& password, std::string& message);
    bool addEntry(const std::string& appName, const std::string& username, const std::string& password, std::string& message);
    bool updateEntry(const std::string& entryId, const std::string& appName, const std::string& username, const std::string& password, std::string& message);
    std::vector<PasswordEntry> getAllEntries(std::string& message);
    std::vector<PasswordEntry> searchEntries(const std::string& query, std::string& message);

private:
    std::vector<PasswordEntry> mapEntries(const std::vector<password_manager_backend::VaultEntryView>& entries) const;

    password_manager_backend::InMemoryUserRepository userRepository_;
    password_manager_backend::InMemoryVaultRepository vaultRepository_;
    password_manager_backend::InMemoryAuditRepository auditRepository_;
    password_manager_backend::CryptoService cryptoService_;
    password_manager_backend::MfaService mfaService_;
    password_manager_backend::AuditService auditService_;
    password_manager_backend::SearchService searchService_;
    password_manager_backend::AuthService authService_;
    password_manager_backend::VaultService vaultService_;
};
