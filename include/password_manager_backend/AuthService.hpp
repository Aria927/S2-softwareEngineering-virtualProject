#pragma once

#include "password_manager_backend/CryptoService.hpp"
#include "password_manager_backend/Repositories.hpp"

namespace password_manager_backend {

class AuthService {
public:
    AuthService(UserRepository& userRepository, CryptoService& cryptoService);

    OperationResult registerUser(const RegisterRequest& request);
    OperationResult login(const LoginRequest& request);
    OperationResult logout();
    OperationResult verifyMasterPassword(const std::string& masterPassword);
    const SessionState& getSessionState() const;

private:
    static std::string createIdentifier();
    static bool isStrongEnough(const std::string& secret);

    UserRepository& userRepository_;
    CryptoService& cryptoService_;
    SessionState session_;
};

}  // namespace password_manager_backend
