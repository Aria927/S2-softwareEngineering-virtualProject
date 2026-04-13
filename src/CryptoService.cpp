#include "password_manager_backend/CryptoService.hpp"

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonKeyDerivation.h>

#include <stdexcept>
#include <vector>

namespace password_manager_backend {
namespace {

// Separate constants keep the security-sensitive values visible and easy to tune.
constexpr std::size_t kHashLength = CC_SHA256_DIGEST_LENGTH;
constexpr std::size_t kEncryptionKeyLength = kCCKeySizeAES256;
constexpr std::size_t kIvLength = kCCBlockSizeAES128;
constexpr int kHashRounds = 120000;
constexpr int kKeyRounds = 150000;

void throwOnCryptorFailure(CCCryptorStatus status, const char* operation) {
    if (status != kCCSuccess) {
        throw std::runtime_error(std::string(operation) + " failed");
    }
}

}  // namespace

std::string CryptoService::generateRandomBytes(std::size_t length) const {
    std::string bytes(length, '\0');
    arc4random_buf(bytes.data(), length);
    return bytes;
}

std::string CryptoService::hashSecret(const std::string& secret, const std::string& salt) const {
    return pbkdf2Sha256(secret, salt, kHashRounds, kHashLength);
}

std::string CryptoService::deriveVaultKey(const std::string& masterPassword, const std::string& salt) const {
    return pbkdf2Sha256(masterPassword, salt, kKeyRounds, kEncryptionKeyLength);
}

bool CryptoService::verifySecret(
    const std::string& secret,
    const std::string& salt,
    const std::string& expectedHash) const {
    return hashSecret(secret, salt) == expectedHash;
}

EncryptedBlob CryptoService::encrypt(const std::string& plainText, const std::string& keyMaterial) const {
    // Each encrypted password gets its own salt and IV so identical passwords do not produce identical stored values.
    const std::string salt = generateRandomBytes(16);
    const std::string iv = generateRandomBytes(kIvLength);
    const std::string encryptionKey = deriveVaultKey(keyMaterial, salt);

    std::vector<char> cipherBuffer(plainText.size() + kCCBlockSizeAES128);
    std::size_t bytesEncrypted = 0;

    const CCCryptorStatus status = CCCrypt(
        kCCEncrypt,
        kCCAlgorithmAES,
        kCCOptionPKCS7Padding,
        encryptionKey.data(),
        encryptionKey.size(),
        iv.data(),
        plainText.data(),
        plainText.size(),
        cipherBuffer.data(),
        cipherBuffer.size(),
        &bytesEncrypted);

    throwOnCryptorFailure(status, "Encryption");

    return EncryptedBlob{
        std::string(cipherBuffer.data(), bytesEncrypted),
        salt,
        iv,
    };
}

std::string CryptoService::decrypt(const EncryptedBlob& blob, const std::string& keyMaterial) const {
    // Decryption derives the same per-entry key using the stored salt and the active master password.
    const std::string encryptionKey = deriveVaultKey(keyMaterial, blob.salt);
    std::vector<char> plainBuffer(blob.cipherText.size() + kCCBlockSizeAES128);
    std::size_t bytesDecrypted = 0;

    const CCCryptorStatus status = CCCrypt(
        kCCDecrypt,
        kCCAlgorithmAES,
        kCCOptionPKCS7Padding,
        encryptionKey.data(),
        encryptionKey.size(),
        blob.iv.data(),
        blob.cipherText.data(),
        blob.cipherText.size(),
        plainBuffer.data(),
        plainBuffer.size(),
        &bytesDecrypted);

    throwOnCryptorFailure(status, "Decryption");

    return std::string(plainBuffer.data(), bytesDecrypted);
}

std::string CryptoService::pbkdf2Sha256(
    const std::string& password,
    const std::string& salt,
    int rounds,
    std::size_t length) const {
    // A derived byte string is returned directly because the backend stores and compares binary-safe strings internally.
    std::string derived(length, '\0');

    const int status = CCKeyDerivationPBKDF(
        kCCPBKDF2,
        password.data(),
        password.size(),
        reinterpret_cast<const std::uint8_t*>(salt.data()),
        salt.size(),
        kCCPRFHmacAlgSHA256,
        rounds,
        reinterpret_cast<std::uint8_t*>(derived.data()),
        derived.size());

    if (status != kCCSuccess) {
        throw std::runtime_error("PBKDF2 derivation failed");
    }

    return derived;
}

}  // namespace password_manager_backend
