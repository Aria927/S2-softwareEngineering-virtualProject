#include "password_manager_backend/CryptoService.hpp"

#if defined(__APPLE__)
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonKeyDerivation.h>
#elif defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")
#endif

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace password_manager_backend {
namespace {

// Separate constants keep the security-sensitive values visible and easy to tune.
constexpr std::size_t kHashLength = 32;
constexpr std::size_t kEncryptionKeyLength = 32;
constexpr std::size_t kIvLength = 16;
constexpr std::size_t kAesBlockSize = 16;
constexpr int kHashRounds = 120000;
constexpr int kKeyRounds = 150000;

#if defined(__APPLE__)
void throwOnCryptorFailure(CCCryptorStatus status, const char* operation) {
    if (status != kCCSuccess) {
        throw std::runtime_error(std::string(operation) + " failed");
    }
}
#elif defined(_WIN32)
void throwOnNtFailure(NTSTATUS status, const char* operation) {
    if (status < 0) {
        throw std::runtime_error(std::string(operation) + " failed");
    }
}
#endif

}  // namespace

std::string CryptoService::generateRandomBytes(std::size_t length) const {
    std::string bytes(length, '\0');

#if defined(__APPLE__)
    arc4random_buf(bytes.data(), length);
#elif defined(_WIN32)
    throwOnNtFailure(
        BCryptGenRandom(
            nullptr,
            reinterpret_cast<PUCHAR>(bytes.data()),
            static_cast<ULONG>(bytes.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG),
        "Random byte generation");
#else
    static_assert(true, "Unsupported platform for CryptoService");
#endif

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

#if defined(__APPLE__)
    std::vector<char> cipherBuffer(plainText.size() + kAesBlockSize);
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
#elif defined(_WIN32)
    BCRYPT_ALG_HANDLE algorithmHandle = nullptr;
    BCRYPT_KEY_HANDLE keyHandle = nullptr;

    throwOnNtFailure(
        BCryptOpenAlgorithmProvider(&algorithmHandle, BCRYPT_AES_ALGORITHM, nullptr, 0),
        "AES provider open");

    throwOnNtFailure(
        BCryptSetProperty(
            algorithmHandle,
            BCRYPT_CHAINING_MODE,
            reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_CBC)),
            static_cast<ULONG>((wcslen(BCRYPT_CHAIN_MODE_CBC) + 1) * sizeof(wchar_t)),
            0),
        "AES chaining mode setup");

    ULONG objectLength = 0;
    ULONG ignored = 0;
    throwOnNtFailure(
        BCryptGetProperty(
            algorithmHandle,
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&objectLength),
            sizeof(objectLength),
            &ignored,
            0),
        "AES object length query");

    std::vector<unsigned char> keyObject(objectLength);
    throwOnNtFailure(
        BCryptGenerateSymmetricKey(
            algorithmHandle,
            &keyHandle,
            keyObject.data(),
            static_cast<ULONG>(keyObject.size()),
            reinterpret_cast<PUCHAR>(const_cast<char*>(encryptionKey.data())),
            static_cast<ULONG>(encryptionKey.size()),
            0),
        "AES key generation");

    std::vector<unsigned char> ivBuffer(iv.begin(), iv.end());
    ULONG encryptedSize = 0;
    throwOnNtFailure(
        BCryptEncrypt(
            keyHandle,
            reinterpret_cast<PUCHAR>(const_cast<char*>(plainText.data())),
            static_cast<ULONG>(plainText.size()),
            nullptr,
            ivBuffer.data(),
            static_cast<ULONG>(ivBuffer.size()),
            nullptr,
            0,
            &encryptedSize,
            BCRYPT_BLOCK_PADDING),
        "Encryption size calculation");

    std::vector<char> cipherBuffer(encryptedSize);
    ivBuffer.assign(iv.begin(), iv.end());
    throwOnNtFailure(
        BCryptEncrypt(
            keyHandle,
            reinterpret_cast<PUCHAR>(const_cast<char*>(plainText.data())),
            static_cast<ULONG>(plainText.size()),
            nullptr,
            ivBuffer.data(),
            static_cast<ULONG>(ivBuffer.size()),
            reinterpret_cast<PUCHAR>(cipherBuffer.data()),
            static_cast<ULONG>(cipherBuffer.size()),
            &encryptedSize,
            BCRYPT_BLOCK_PADDING),
        "Encryption");

    BCryptDestroyKey(keyHandle);
    BCryptCloseAlgorithmProvider(algorithmHandle, 0);

    return EncryptedBlob{
        std::string(cipherBuffer.data(), encryptedSize),
        salt,
        iv,
    };
#else
    static_assert(true, "Unsupported platform for CryptoService");
#endif
}

std::string CryptoService::decrypt(const EncryptedBlob& blob, const std::string& keyMaterial) const {
    // Decryption derives the same per-entry key using the stored salt and the active master password.
    const std::string encryptionKey = deriveVaultKey(keyMaterial, blob.salt);

#if defined(__APPLE__)
    std::vector<char> plainBuffer(blob.cipherText.size() + kAesBlockSize);
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
#elif defined(_WIN32)
    BCRYPT_ALG_HANDLE algorithmHandle = nullptr;
    BCRYPT_KEY_HANDLE keyHandle = nullptr;

    throwOnNtFailure(
        BCryptOpenAlgorithmProvider(&algorithmHandle, BCRYPT_AES_ALGORITHM, nullptr, 0),
        "AES provider open");

    throwOnNtFailure(
        BCryptSetProperty(
            algorithmHandle,
            BCRYPT_CHAINING_MODE,
            reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_CBC)),
            static_cast<ULONG>((wcslen(BCRYPT_CHAIN_MODE_CBC) + 1) * sizeof(wchar_t)),
            0),
        "AES chaining mode setup");

    ULONG objectLength = 0;
    ULONG ignored = 0;
    throwOnNtFailure(
        BCryptGetProperty(
            algorithmHandle,
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&objectLength),
            sizeof(objectLength),
            &ignored,
            0),
        "AES object length query");

    std::vector<unsigned char> keyObject(objectLength);
    throwOnNtFailure(
        BCryptGenerateSymmetricKey(
            algorithmHandle,
            &keyHandle,
            keyObject.data(),
            static_cast<ULONG>(keyObject.size()),
            reinterpret_cast<PUCHAR>(const_cast<char*>(encryptionKey.data())),
            static_cast<ULONG>(encryptionKey.size()),
            0),
        "AES key generation");

    std::vector<unsigned char> ivBuffer(blob.iv.begin(), blob.iv.end());
    ULONG plainSize = 0;
    throwOnNtFailure(
        BCryptDecrypt(
            keyHandle,
            reinterpret_cast<PUCHAR>(const_cast<char*>(blob.cipherText.data())),
            static_cast<ULONG>(blob.cipherText.size()),
            nullptr,
            ivBuffer.data(),
            static_cast<ULONG>(ivBuffer.size()),
            nullptr,
            0,
            &plainSize,
            BCRYPT_BLOCK_PADDING),
        "Decryption size calculation");

    std::vector<char> plainBuffer(plainSize);
    ivBuffer.assign(blob.iv.begin(), blob.iv.end());
    throwOnNtFailure(
        BCryptDecrypt(
            keyHandle,
            reinterpret_cast<PUCHAR>(const_cast<char*>(blob.cipherText.data())),
            static_cast<ULONG>(blob.cipherText.size()),
            nullptr,
            ivBuffer.data(),
            static_cast<ULONG>(ivBuffer.size()),
            reinterpret_cast<PUCHAR>(plainBuffer.data()),
            static_cast<ULONG>(plainBuffer.size()),
            &plainSize,
            BCRYPT_BLOCK_PADDING),
        "Decryption");

    BCryptDestroyKey(keyHandle);
    BCryptCloseAlgorithmProvider(algorithmHandle, 0);

    return std::string(plainBuffer.data(), plainSize);
#else
    static_assert(true, "Unsupported platform for CryptoService");
#endif
}

std::string CryptoService::pbkdf2Sha256(
    const std::string& password,
    const std::string& salt,
    int rounds,
    std::size_t length) const {
    // A derived byte string is returned directly because the backend stores and compares binary-safe strings internally.
    std::string derived(length, '\0');

#if defined(__APPLE__)
    const int status = CCKeyDerivationPBKDF(
        kCCPBKDF2,
        password.data(),
        static_cast<int>(password.size()),
        reinterpret_cast<const std::uint8_t*>(salt.data()),
        static_cast<int>(salt.size()),
        kCCPRFHmacAlgSHA256,
        rounds,
        reinterpret_cast<std::uint8_t*>(derived.data()),
        static_cast<int>(derived.size()));

    if (status != kCCSuccess) {
        throw std::runtime_error("PBKDF2 derivation failed");
    }
#elif defined(_WIN32)
    BCRYPT_ALG_HANDLE algorithmHandle = nullptr;
    throwOnNtFailure(
        BCryptOpenAlgorithmProvider(&algorithmHandle, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG),
        "SHA256 provider open");

    throwOnNtFailure(
        BCryptDeriveKeyPBKDF2(
            algorithmHandle,
            reinterpret_cast<PUCHAR>(const_cast<char*>(password.data())),
            static_cast<ULONG>(password.size()),
            reinterpret_cast<PUCHAR>(const_cast<char*>(salt.data())),
            static_cast<ULONG>(salt.size()),
            static_cast<ULONGLONG>(rounds),
            reinterpret_cast<PUCHAR>(derived.data()),
            static_cast<ULONG>(derived.size()),
            0),
        "PBKDF2 derivation");

    BCryptCloseAlgorithmProvider(algorithmHandle, 0);
#else
    static_assert(true, "Unsupported platform for CryptoService");
#endif

    return derived;
}

}  // namespace password_manager_backend
