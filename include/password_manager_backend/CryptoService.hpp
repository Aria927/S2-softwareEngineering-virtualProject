#pragma once

#include "password_manager_backend/Models.hpp"

#include <string>

namespace password_manager_backend {

class CryptoService {
public:
    std::string generateRandomBytes(std::size_t length) const;
    std::string hashSecret(const std::string& secret, const std::string& salt) const;
    std::string deriveVaultKey(const std::string& masterPassword, const std::string& salt) const;
    bool verifySecret(const std::string& secret, const std::string& salt, const std::string& expectedHash) const;

    EncryptedBlob encrypt(const std::string& plainText, const std::string& keyMaterial) const;
    std::string decrypt(const EncryptedBlob& blob, const std::string& keyMaterial) const;

private:
    std::string pbkdf2Sha256(
        const std::string& password,
        const std::string& salt,
        int rounds,
        std::size_t length) const;
};

}  // namespace password_manager_backend
