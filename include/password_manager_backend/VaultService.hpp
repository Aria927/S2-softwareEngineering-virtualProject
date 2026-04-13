#pragma once

#include "password_manager_backend/AuditService.hpp"
#include "password_manager_backend/AuthService.hpp"
#include "password_manager_backend/CryptoService.hpp"
#include "password_manager_backend/Repositories.hpp"
#include "password_manager_backend/SearchService.hpp"

namespace password_manager_backend {

// Coordinates vault operations and ensures only authenticated, unlocked sessions can access secrets.
class VaultService {
public:
    VaultService(
        VaultRepository& vaultRepository,
        AuthService& authService,
        CryptoService& cryptoService,
        SearchService& searchService,
        AuditService& auditService);

    Result<VaultEntryView> createVaultEntry(const VaultEntryInput& input);
    Result<std::vector<VaultEntryView>> getVaultEntries() const;
    Result<VaultEntryView> updateVaultEntry(const std::string& entryId, const VaultEntryInput& input);
    OperationResult deleteVaultEntry(const std::string& entryId);
    Result<std::vector<VaultEntryView>> searchVaultEntries(const SearchQuery& query) const;

private:
    // The backend owns entry identifiers so repository implementations can stay simple.
    static std::string createIdentifier();
    static OperationResult validateEntryInput(const VaultEntryInput& input);
    Result<VaultEntryView> toView(const VaultEntry& entry) const;
    Result<std::vector<VaultEntryView>> listEntries() const;

    VaultRepository& vaultRepository_;
    AuthService& authService_;
    CryptoService& cryptoService_;
    SearchService& searchService_;
    AuditService& auditService_;
};

}  // namespace password_manager_backend
