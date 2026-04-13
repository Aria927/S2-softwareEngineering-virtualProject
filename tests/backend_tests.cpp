#include "password_manager_backend/AuthService.hpp"
#include "password_manager_backend/Repositories.hpp"
#include "password_manager_backend/SearchService.hpp"
#include "password_manager_backend/VaultService.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace password_manager_backend;

namespace {

// Keeps the test file dependency-free while still failing fast with readable messages.
void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testRegistrationAndDuplicateValidation() {
    // This test proves the backend validates inputs and does not keep account secrets in plaintext.
    InMemoryUserRepository userRepository;
    CryptoService cryptoService;
    AuthService authService(userRepository, cryptoService);

    const auto firstRegistration = authService.registerUser({
        .username = "alice",
        .email = "alice@example.com",
        .password = "password123",
        .masterPassword = "masterpass123",
    });

    expect(firstRegistration.success, "Expected first registration to succeed.");

    const auto duplicateUsername = authService.registerUser({
        .username = "alice",
        .email = "another@example.com",
        .password = "password123",
        .masterPassword = "masterpass123",
    });

    expect(!duplicateUsername.success, "Expected duplicate username to fail.");

    const auto weakPassword = authService.registerUser({
        .username = "bob",
        .email = "bob@example.com",
        .password = "short",
        .masterPassword = "masterpass123",
    });

    expect(!weakPassword.success, "Expected weak password to fail.");

    const auto storedUser = userRepository.findByUsername("alice");
    expect(storedUser.has_value(), "Stored user should exist.");
    expect(storedUser->passwordHash != "password123", "Password should not be stored in plain text.");
    expect(storedUser->masterPasswordHash != "masterpass123", "Master password should not be stored in plain text.");
}

void testLoginUnlockAndLogout() {
    // Login and vault unlock are tested separately because they represent two different security gates.
    InMemoryUserRepository userRepository;
    CryptoService cryptoService;
    AuthService authService(userRepository, cryptoService);

    authService.registerUser({
        .username = "alice",
        .email = "alice@example.com",
        .password = "password123",
        .masterPassword = "masterpass123",
    });

    const auto invalidLogin = authService.login({
        .username = "alice",
        .password = "wrongpassword",
    });

    expect(!invalidLogin.success, "Expected invalid login to fail.");

    const auto validLogin = authService.login({
        .username = "alice",
        .password = "password123",
    });

    expect(validLogin.success, "Expected valid login to succeed.");
    expect(authService.getSessionState().isAuthenticated, "Expected session to be authenticated.");
    expect(!authService.getSessionState().isVaultUnlocked, "Vault should stay locked until master password is checked.");

    const auto invalidUnlock = authService.verifyMasterPassword("wrongmaster");
    expect(!invalidUnlock.success, "Expected invalid master password to fail.");

    const auto validUnlock = authService.verifyMasterPassword("masterpass123");
    expect(validUnlock.success, "Expected valid master password to unlock vault.");
    expect(authService.getSessionState().isVaultUnlocked, "Vault should be unlocked.");

    const auto logoutResult = authService.logout();
    expect(logoutResult.success, "Expected logout to succeed.");
    expect(!authService.getSessionState().isAuthenticated, "Expected session to be cleared on logout.");
}

void testVaultCrudAndSearch() {
    // This covers the main backend flow the GUI will eventually call into.
    InMemoryUserRepository userRepository;
    InMemoryVaultRepository vaultRepository;
    CryptoService cryptoService;
    SearchService searchService;
    AuthService authService(userRepository, cryptoService);
    VaultService vaultService(vaultRepository, authService, cryptoService, searchService);

    authService.registerUser({
        .username = "alice",
        .email = "alice@example.com",
        .password = "password123",
        .masterPassword = "masterpass123",
    });

    authService.login({
        .username = "alice",
        .password = "password123",
    });

    auto lockedAccess = vaultService.getVaultEntries();
    expect(!lockedAccess.success, "Expected vault access to fail when still locked.");

    authService.verifyMasterPassword("masterpass123");

    const auto createdEntry = vaultService.createVaultEntry({
        .title = "GitHub",
        .site = "github.com",
        .username = "alice-dev",
        .password = "secret-token",
        .notes = "Primary source control account",
        .tags = {"work", "git"},
    });

    expect(createdEntry.success, "Expected entry creation to succeed.");
    expect(createdEntry.value.has_value(), "Created entry should be returned.");
    expect(createdEntry.value->password == "secret-token", "Vault should return decrypted passwords after unlock.");

    const auto repositoryEntry = vaultRepository.findById(createdEntry.value->id);
    expect(repositoryEntry.has_value(), "Stored vault entry should exist.");
    expect(repositoryEntry->encryptedPassword != "secret-token", "Stored vault password must be encrypted.");

    const auto updatedEntry = vaultService.updateVaultEntry(createdEntry.value->id, {
        .title = "GitHub Personal",
        .site = "github.com",
        .username = "alice-dev",
        .password = "new-secret-token",
        .notes = "Updated account",
        .tags = {"work", "source-control"},
    });

    expect(updatedEntry.success, "Expected update to succeed.");
    expect(updatedEntry.value->password == "new-secret-token", "Updated decrypted password should be returned.");

    const auto allEntries = vaultService.getVaultEntries();
    expect(allEntries.success, "Expected listing entries to succeed.");
    expect(allEntries.value->size() == 1, "Expected one stored entry.");

    const auto searchResults = vaultService.searchVaultEntries({.text = "source"});
    expect(searchResults.success, "Expected search to succeed.");
    expect(searchResults.value->size() == 1, "Expected search to match updated tag.");

    const auto emptySearch = vaultService.searchVaultEntries({.text = ""});
    expect(emptySearch.success, "Expected empty search to succeed.");
    expect(emptySearch.value->size() == 1, "Expected empty search to return all entries.");

    const auto deleteResult = vaultService.deleteVaultEntry(createdEntry.value->id);
    expect(deleteResult.success, "Expected delete to succeed.");

    const auto postDeleteEntries = vaultService.getVaultEntries();
    expect(postDeleteEntries.success, "Expected listing entries after delete to succeed.");
    expect(postDeleteEntries.value->empty(), "Expected entry list to be empty after delete.");
}

}  // namespace

int main() {
    // A tiny manual test runner keeps setup simple and portable for this student project.
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"Registration and duplicate validation", testRegistrationAndDuplicateValidation},
        {"Login, unlock, and logout", testLoginUnlockAndLogout},
        {"Vault CRUD and search", testVaultCrudAndSearch},
    };

    for (const auto& [name, test] : tests) {
        test();
        std::cout << "[PASS] " << name << '\n';
    }

    std::cout << "All backend tests passed.\n";
    return 0;
}
