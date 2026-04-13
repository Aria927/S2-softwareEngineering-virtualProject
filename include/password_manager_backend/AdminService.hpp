#pragma once

#include "password_manager_backend/AuditService.hpp"
#include "password_manager_backend/AuthService.hpp"

namespace password_manager_backend {

// Exposes admin-only backend actions without pulling any GUI-specific panel logic into the service layer.
class AdminService {
public:
    AdminService(
        UserRepository& userRepository,
        CryptoService& cryptoService,
        AuditService& auditService,
        AuthService& authService,
        std::string recoveryKey);

    Result<std::vector<AdminUserView>> listUsers() const;
    Result<std::vector<AuditEvent>> getAuditLog() const;
    OperationResult resetUserMasterPassword(const std::string& targetUserId, const std::string& newMasterPassword);

private:
    OperationResult ensureAdminSession() const;

    UserRepository& userRepository_;
    CryptoService& cryptoService_;
    AuditService& auditService_;
    AuthService& authService_;
    std::string recoveryKey_;
};

}  // namespace password_manager_backend
