#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace password_manager_backend {

// Stores encrypted password data and the metadata needed to decrypt it later.
struct EncryptedBlob {
    std::string cipherText;
    std::string salt;
    std::string iv;
};

// Represents a locally registered user and their stored authentication material.
struct UserAccount {
    std::string id;
    std::string username;
    std::string email;
    std::string passwordHash;
    std::string passwordSalt;
    std::string masterPasswordHash;
    std::string masterPasswordSalt;
};

// Represents a vault record as stored by the persistence layer.
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

// Carries plaintext input from the GUI before the service layer validates and encrypts it.
struct VaultEntryInput {
    std::string title;
    std::string site;
    std::string username;
    std::string password;
    std::string notes;
    std::vector<std::string> tags;
};

// Represents a decrypted entry that is safe to pass back to the GUI after unlock.
struct VaultEntryView {
    std::string id;
    std::string title;
    std::string site;
    std::string username;
    std::string password;
    std::string notes;
    std::vector<std::string> tags;
};

// Keeps search behaviour simple and extensible as more filters are added later.
struct SearchQuery {
    std::string text;
};

// Tracks whether a user is signed in and whether the vault is currently unlocked.
struct SessionState {
    bool isAuthenticated{false};
    bool isVaultUnlocked{false};
    std::optional<std::string> userId;
    std::optional<std::string> username;
    std::optional<std::string> activeEncryptionKey;
};

// Input model for user registration.
struct RegisterRequest {
    std::string username;
    std::string email;
    std::string password;
    std::string masterPassword;
};

// Input model for user login.
struct LoginRequest {
    std::string username;
    std::string password;
};

// Simple success/failure result for operations that do not return data.
struct OperationResult {
    bool success{false};
    std::string message;
};

// Generic result wrapper for service methods that return both status and data.
template <typename T>
struct Result {
    bool success{false};
    std::string message;
    std::optional<T> value;
};

}  // namespace password_manager_backend
