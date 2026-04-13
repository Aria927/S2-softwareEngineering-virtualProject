#include "password_manager_backend/AuthService.hpp"

#include <algorithm>
#include <cctype>
#include <random>

namespace password_manager_backend {
namespace {

// Normalises basic text input before validation and lookup.
std::string trim(const std::string& value) {
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });

    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();

    if (begin >= end) {
        return {};
    }

    return std::string(begin, end);
}

}  // namespace

AuthService::AuthService(
    UserRepository& userRepository,
    CryptoService& cryptoService,
    MfaService& mfaService,
    AuditService& auditService,
    std::string recoveryKey)
    : userRepository_(userRepository),
      cryptoService_(cryptoService),
      mfaService_(mfaService),
      auditService_(auditService),
      recoveryKey_(std::move(recoveryKey)) {}

OperationResult AuthService::registerUser(const RegisterRequest& request) {
    const std::string username = trim(request.username);
    const std::string email = trim(request.email);

    if (username.empty() || email.empty()) {
        return {false, "Username and email are required."};
    }

    if (!isStrongEnough(request.password)) {
        return {false, "Account password must be at least 8 characters long."};
    }

    if (!isStrongEnough(request.masterPassword)) {
        return {false, "Master password must be at least 8 characters long."};
    }

    if (userRepository_.findByUsername(username).has_value()) {
        return {false, "That username is already registered."};
    }

    if (userRepository_.findByEmail(email).has_value()) {
        return {false, "That email is already registered."};
    }

    // Account and master secrets are stored independently because they serve different security roles.
    const std::string passwordSalt = cryptoService_.generateRandomBytes(16);
    const std::string masterPasswordSalt = cryptoService_.generateRandomBytes(16);
    const std::string vaultKey = cryptoService_.generateRandomBytes(32);

    UserAccount user{
        .id = createIdentifier(),
        .username = username,
        .email = email,
        .passwordHash = cryptoService_.hashSecret(request.password, passwordSalt),
        .passwordSalt = passwordSalt,
        .masterPasswordHash = cryptoService_.hashSecret(request.masterPassword, masterPasswordSalt),
        .masterPasswordSalt = masterPasswordSalt,
        .role = request.isAdmin ? UserRole::Admin : UserRole::Standard,
        .mfaEnabled = false,
        .encryptedMfaSecret = std::nullopt,
        .masterWrappedVaultKey = cryptoService_.encrypt(vaultKey, request.masterPassword),
        .passwordWrappedVaultKey = cryptoService_.encrypt(vaultKey, request.password),
        .recoveryWrappedVaultKey = cryptoService_.encrypt(vaultKey, recoveryKey_),
    };

    userRepository_.save(user);
    auditService_.recordEvent(user.id, user.username, "REGISTER", "User account created.");
    return {true, "User account created."};
}

OperationResult AuthService::login(const LoginRequest& request) {
    const auto user = userRepository_.findByUsername(trim(request.username));
    if (!user.has_value()) {
        return {false, "Invalid username or password."};
    }

    const bool isValid = cryptoService_.verifySecret(
        request.password,
        user->passwordSalt,
        user->passwordHash);

    if (!isValid) {
        auditService_.recordEvent("", trim(request.username), "LOGIN_FAILED", "Password verification failed.");
        return {false, "Invalid username or password."};
    }

    if (user->mfaEnabled) {
        const auto secretResult = mfaService_.decryptSecret(*user, request.password);
        if (!secretResult.success || !secretResult.value.has_value()) {
            return {false, "MFA could not be prepared for this session."};
        }

        // MFA-enabled accounts pause after password verification until the one-time code is confirmed.
        session_ = SessionState{
            .isAuthenticated = false,
            .isVaultUnlocked = false,
            .isMfaRequired = true,
            .isAdmin = false,
            .userId = std::nullopt,
            .username = std::nullopt,
            .activeEncryptionKey = std::nullopt,
            .pendingUserId = user->id,
            .pendingUsername = user->username,
            .pendingMfaSecret = *secretResult.value,
            .pendingIsAdmin = user->role == UserRole::Admin,
        };

        auditService_.recordEvent(user->id, user->username, "LOGIN_MFA_REQUIRED", "Password verified. Awaiting MFA.");
        return {true, "MFA required before login completes."};
    }

    // Logging in authenticates the user but intentionally keeps the vault locked until the master password is verified.
    session_ = SessionState{
        .isAuthenticated = true,
        .isVaultUnlocked = false,
        .isMfaRequired = false,
        .isAdmin = user->role == UserRole::Admin,
        .userId = user->id,
        .username = user->username,
        .activeEncryptionKey = std::nullopt,
        .pendingUserId = std::nullopt,
        .pendingUsername = std::nullopt,
        .pendingMfaSecret = std::nullopt,
        .pendingIsAdmin = false,
    };

    auditService_.recordEvent(user->id, user->username, "LOGIN_SUCCESS", "User logged in.");
    return {true, "Login successful."};
}

OperationResult AuthService::completeMfaLogin(const std::string& mfaCode) {
    if (!session_.isMfaRequired || !session_.pendingUserId.has_value() || !session_.pendingUsername.has_value() ||
        !session_.pendingMfaSecret.has_value()) {
        return {false, "There is no MFA challenge waiting to be completed."};
    }

    if (!mfaService_.verifyCode(*session_.pendingMfaSecret, mfaCode)) {
        auditService_.recordEvent(*session_.pendingUserId, *session_.pendingUsername, "MFA_FAILED", "Incorrect MFA code.");
        return {false, "Invalid MFA code."};
    }

    session_.isAuthenticated = true;
    session_.isVaultUnlocked = false;
    session_.isMfaRequired = false;
    session_.isAdmin = session_.pendingIsAdmin;
    session_.userId = session_.pendingUserId;
    session_.username = session_.pendingUsername;
    session_.pendingUserId.reset();
    session_.pendingUsername.reset();
    session_.pendingMfaSecret.reset();
    session_.pendingIsAdmin = false;

    auditService_.recordEvent(*session_.userId, *session_.username, "LOGIN_SUCCESS", "User logged in after MFA.");
    return {true, "MFA verification complete."};
}

OperationResult AuthService::logout() {
    if (session_.userId.has_value() && session_.username.has_value()) {
        auditService_.recordEvent(*session_.userId, *session_.username, "LOGOUT", "User logged out.");
    } else if (session_.pendingUserId.has_value() && session_.pendingUsername.has_value()) {
        auditService_.recordEvent(*session_.pendingUserId, *session_.pendingUsername, "LOGOUT", "Pending MFA session cleared.");
    }

    session_ = SessionState{};
    return {true, "Logout successful."};
}

OperationResult AuthService::verifyMasterPassword(const std::string& masterPassword) {
    if (!session_.isAuthenticated || !session_.userId.has_value()) {
        return {false, "You must log in before unlocking the vault."};
    }

    const auto user = userRepository_.findById(*session_.userId);
    if (!user.has_value()) {
        return {false, "Active user could not be found."};
    }

    const bool isValid = cryptoService_.verifySecret(
        masterPassword,
        user->masterPasswordSalt,
        user->masterPasswordHash);

    if (!isValid) {
        session_.isVaultUnlocked = false;
        session_.activeEncryptionKey.reset();
        return {false, "Master password verification failed."};
    }

    // The master password unwraps the real vault key, which allows master password resets without re-encrypting each entry.
    session_.isVaultUnlocked = true;
    session_.activeEncryptionKey = cryptoService_.decrypt(user->masterWrappedVaultKey, masterPassword);
    auditService_.recordEvent(user->id, user->username, "VAULT_UNLOCK", "Master password verified.");
    return {true, "Vault unlocked."};
}

Result<std::string> AuthService::enableMfaForCurrentUser(const std::string& accountPassword) {
    if (!session_.isAuthenticated || !session_.userId.has_value() || !session_.username.has_value()) {
        return {false, "You must be logged in to enable MFA.", std::nullopt};
    }

    auto user = userRepository_.findById(*session_.userId);
    if (!user.has_value()) {
        return {false, "Active user could not be found.", std::nullopt};
    }

    if (!cryptoService_.verifySecret(accountPassword, user->passwordSalt, user->passwordHash)) {
        return {false, "Account password verification failed.", std::nullopt};
    }

    auto secretResult = mfaService_.createAndStoreSecret(*user, accountPassword);
    if (!secretResult.success) {
        return secretResult;
    }

    userRepository_.update(*user);
    auditService_.recordEvent(user->id, user->username, "MFA_ENABLED", "MFA enabled for account.");
    return secretResult;
}

OperationResult AuthService::disableMfaForCurrentUser(const std::string& accountPassword) {
    if (!session_.isAuthenticated || !session_.userId.has_value() || !session_.username.has_value()) {
        return {false, "You must be logged in to disable MFA."};
    }

    auto user = userRepository_.findById(*session_.userId);
    if (!user.has_value()) {
        return {false, "Active user could not be found."};
    }

    if (!cryptoService_.verifySecret(accountPassword, user->passwordSalt, user->passwordHash)) {
        return {false, "Account password verification failed."};
    }

    const auto clearResult = mfaService_.clearSecret(*user);
    if (!clearResult.success) {
        return clearResult;
    }

    userRepository_.update(*user);
    auditService_.recordEvent(user->id, user->username, "MFA_DISABLED", "MFA disabled for account.");
    return clearResult;
}

OperationResult AuthService::resetOwnMasterPassword(const std::string& accountPassword, const std::string& newMasterPassword) {
    if (!session_.isAuthenticated || !session_.userId.has_value() || !session_.username.has_value()) {
        return {false, "You must be logged in to reset the master password."};
    }

    if (!isStrongEnough(newMasterPassword)) {
        return {false, "New master password must be at least 8 characters long."};
    }

    auto user = userRepository_.findById(*session_.userId);
    if (!user.has_value()) {
        return {false, "Active user could not be found."};
    }

    if (!cryptoService_.verifySecret(accountPassword, user->passwordSalt, user->passwordHash)) {
        return {false, "Account password verification failed."};
    }

    // The password-wrapped vault key lets a user reset a forgotten master password after normal login verification.
    const std::string vaultKey = cryptoService_.decrypt(user->passwordWrappedVaultKey, accountPassword);
    user->masterPasswordSalt = cryptoService_.generateRandomBytes(16);
    user->masterPasswordHash = cryptoService_.hashSecret(newMasterPassword, user->masterPasswordSalt);
    user->masterWrappedVaultKey = cryptoService_.encrypt(vaultKey, newMasterPassword);
    userRepository_.update(*user);

    if (session_.isVaultUnlocked) {
        session_.activeEncryptionKey = vaultKey;
    }

    auditService_.recordEvent(user->id, user->username, "MASTER_PASSWORD_RESET_SELF_SERVICE", "User reset their own master password.");
    return {true, "Master password reset successfully."};
}

const SessionState& AuthService::getSessionState() const {
    return session_;
}

std::string AuthService::createIdentifier() {
    // A short hex identifier is enough for local object identity in this student project.
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

bool AuthService::isStrongEnough(const std::string& secret) {
    // This keeps validation intentionally simple; richer password rules can be added later without changing callers.
    return secret.size() >= 8;
}

}  // namespace password_manager_backend
