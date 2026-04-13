#pragma once

#include "password_manager_backend/CryptoService.hpp"
#include "password_manager_backend/Models.hpp"

#include <chrono>

namespace password_manager_backend {

// Provides a lightweight local MFA mechanism suitable for a desktop-only student project.
class MfaService {
public:
    explicit MfaService(CryptoService& cryptoService);

    Result<std::string> createAndStoreSecret(UserAccount& user, const std::string& accountPassword) const;
    OperationResult clearSecret(UserAccount& user) const;
    Result<std::string> decryptSecret(const UserAccount& user, const std::string& accountPassword) const;
    bool verifyCode(
        const std::string& secret,
        const std::string& submittedCode,
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const;

    // This remains public so tests and a future GUI setup flow can generate the expected current code.
    std::string generateCurrentCode(
        const std::string& secret,
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const;

private:
    std::string generateSecret() const;
    std::string generateCodeForStep(const std::string& secret, std::int64_t step) const;

    CryptoService& cryptoService_;
};

}  // namespace password_manager_backend
