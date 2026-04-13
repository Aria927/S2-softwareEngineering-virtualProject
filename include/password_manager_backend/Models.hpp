#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace password_manager_backend {

struct EncryptedBlob {
    std::string cipherText;
    std::string salt;
    std::string iv;
};

struct UserAccount {
    std::string id;
    std::string username;
    std::string email;
    std::string passwordHash;
    std::string passwordSalt;
    std::string masterPasswordHash;
    std::string masterPasswordSalt;
};

struct VaultEntry {
    std::string id;
    std::string userId;
    std::string title;
    std::string site;
    std::string username;
    std::string encryptedPassword;
    std::string encryptionSalt;
    std::string encryptionIv;
    std::string notes;
    std::vector<std::string> tags;
};

struct VaultEntryInput {
    std::string title;
    std::string site;
    std::string username;
    std::string password;
    std::string notes;
    std::vector<std::string> tags;
};

struct VaultEntryView {
    std::string id;
    std::string title;
    std::string site;
    std::string username;
    std::string password;
    std::string notes;
    std::vector<std::string> tags;
};

struct SearchQuery {
    std::string text;
};

struct SessionState {
    bool isAuthenticated{false};
    bool isVaultUnlocked{false};
    std::optional<std::string> userId;
    std::optional<std::string> username;
    std::optional<std::string> activeEncryptionKey;
};

struct RegisterRequest {
    std::string username;
    std::string email;
    std::string password;
    std::string masterPassword;
};

struct LoginRequest {
    std::string username;
    std::string password;
};

struct OperationResult {
    bool success{false};
    std::string message;
};

template <typename T>
struct Result {
    bool success{false};
    std::string message;
    std::optional<T> value;
};

}  // namespace password_manager_backend
