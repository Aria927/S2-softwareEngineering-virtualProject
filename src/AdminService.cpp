#include "password_manager_backend/AdminService.hpp"

namespace password_manager_backend {

AdminService::AdminService(
    UserRepository& userRepository,
    CryptoService& cryptoService,
    AuditService& auditService,
    AuthService& authService,
    std::string recoveryKey)
    : userRepository_(userRepository),
      cryptoService_(cryptoService),
      auditService_(auditService),
      authService_(authService),
      recoveryKey_(std::move(recoveryKey)) {}

Result<std::vector<AdminUserView>> AdminService::listUsers() const {
    const auto adminCheck = ensureAdminSession();
    if (!adminCheck.success) {
        return {false, adminCheck.message, std::nullopt};
    }

    std::vector<AdminUserView> users;
    for (const auto& user : userRepository_.listAll()) {
        users.push_back({
            .id = user.id,
            .username = user.username,
            .email = user.email,
            .isAdmin = user.role == UserRole::Admin,
            .mfaEnabled = user.mfaEnabled,
        });
    }

    return {true, "Users loaded.", users};
}

Result<std::vector<AuditEvent>> AdminService::getAuditLog() const {
    const auto adminCheck = ensureAdminSession();
    if (!adminCheck.success) {
        return {false, adminCheck.message, std::nullopt};
    }

    return {true, "Audit log loaded.", auditService_.getAllEvents()};
}

OperationResult AdminService::resetUserMasterPassword(const std::string& targetUserId, const std::string& newMasterPassword) {
    const auto adminCheck = ensureAdminSession();
    if (!adminCheck.success) {
        return adminCheck;
    }

    if (newMasterPassword.size() < 8) {
        return {false, "New master password must be at least 8 characters long."};
    }

    auto targetUser = userRepository_.findById(targetUserId);
    if (!targetUser.has_value()) {
        return {false, "Target user was not found."};
    }

    // The recovery-wrapped vault key lets an admin reset a forgotten master password without losing vault data.
    const std::string vaultKey = cryptoService_.decrypt(targetUser->recoveryWrappedVaultKey, recoveryKey_);
    const std::string newMasterSalt = cryptoService_.generateRandomBytes(16);

    targetUser->masterPasswordSalt = newMasterSalt;
    targetUser->masterPasswordHash = cryptoService_.hashSecret(newMasterPassword, newMasterSalt);
    targetUser->masterWrappedVaultKey = cryptoService_.encrypt(vaultKey, newMasterPassword);

    userRepository_.update(*targetUser);

    const auto& session = authService_.getSessionState();
    auditService_.recordEvent(*session.userId, *session.username, "ADMIN_RESET_MASTER_PASSWORD", targetUser->username);
    auditService_.recordEvent(targetUser->id, targetUser->username, "MASTER_PASSWORD_RESET_BY_ADMIN", "Reset by admin user.");

    return {true, "User master password reset successfully."};
}

OperationResult AdminService::ensureAdminSession() const {
    const auto& session = authService_.getSessionState();
    if (!session.isAuthenticated || !session.isAdmin || !session.userId.has_value() || !session.username.has_value()) {
        return {false, "Admin privileges are required for this action."};
    }

    return {true, "Admin session verified."};
}

}  // namespace password_manager_backend
