#pragma once

#include "password_manager_backend/CryptoService.hpp"
#include "password_manager_backend/Repositories.hpp"

namespace password_manager_backend {

// Handles account lifecycle and session state, but does not know anything about GUI widgets or storage details.
class AuthService {
public:
    AuthService(UserRepository& userRepository, CryptoService& cryptoService);

    OperationResult registerUser(const RegisterRequest& request);
    OperationResult login(const LoginRequest& request);
    OperationResult logout();
    OperationResult verifyMasterPassword(const std::string& masterPassword);
    const SessionState& getSessionState() const;

private:
    // IDs are generated in the backend so user creation does not depend on database-specific behaviour.
    static std::string createIdentifier();
    static bool isStrongEnough(const std::string& secret);

    UserRepository& userRepository_;
    CryptoService& cryptoService_;
    SessionState session_;
};

}  // namespace password_manager_backend
