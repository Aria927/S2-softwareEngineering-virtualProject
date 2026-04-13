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

AuthService::AuthService(UserRepository& userRepository, CryptoService& cryptoService)
    : userRepository_(userRepository), cryptoService_(cryptoService) {}

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

    UserAccount user{
        .id = createIdentifier(),
        .username = username,
        .email = email,
        .passwordHash = cryptoService_.hashSecret(request.password, passwordSalt),
        .passwordSalt = passwordSalt,
        .masterPasswordHash = cryptoService_.hashSecret(request.masterPassword, masterPasswordSalt),
        .masterPasswordSalt = masterPasswordSalt,
    };

    userRepository_.save(user);
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
        return {false, "Invalid username or password."};
    }

    // Logging in authenticates the user but intentionally keeps the vault locked until the master password is verified.
    session_ = SessionState{
        .isAuthenticated = true,
        .isVaultUnlocked = false,
        .userId = user->id,
        .username = user->username,
        .activeEncryptionKey = std::nullopt,
    };

    return {true, "Login successful."};
}

OperationResult AuthService::logout() {
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

    // The active encryption key stays only in session memory so vault entries can be decrypted during the session.
    session_.isVaultUnlocked = true;
    session_.activeEncryptionKey = masterPassword;
    return {true, "Vault unlocked."};
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
