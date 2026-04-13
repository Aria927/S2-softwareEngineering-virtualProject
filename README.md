# Password Manager Backend

This repository currently contains the backend service layer for a local desktop password manager written in C++. It is designed to be called by a future GUI and connected to a future database implementation, while already providing the main business logic and security behaviour for the application.

## Current backend features
- User registration with duplicate checks and basic password-strength validation.
- Login and logout flow with session tracking.
- Master-password verification before vault access is granted.
- Secure storage design where account credentials are hashed and saved vault passwords are encrypted.
- Vault entry create, read, update, delete, and search operations.
- Multi-factor authentication support for login.
- Self-service master-password reset for logged-in users.
- Admin-only user listing and admin master-password reset.
- Audit logging for registration, login, logout, vault access, and vault management actions.

## Main backend components
- `AuthService` manages registration, login, logout, MFA completion, vault unlock, and self-service master-password reset.
- `VaultService` manages password-entry creation, listing, update, deletion, and secure access checks.
- `CryptoService` handles hashing, key derivation, and encryption/decryption.
- `SearchService` filters decrypted vault entries using user search text.
- `MfaService` creates and validates local one-time MFA codes.
- `AdminService` exposes backend-only admin actions such as listing users and resetting a user’s master password.
- `AuditService` records security and vault events for later inspection.

## Security approach
- Account passwords are stored as hashes and are never recoverable as plaintext.
- Master passwords are also stored as hashes and must be verified before the vault can be unlocked.
- Saved vault passwords are encrypted so they can be shown to the user only after successful unlock.
- A separate vault key is used internally so master passwords can be reset without losing encrypted vault data.
- Audit events are stored without logging decrypted vault passwords.

## Current project state
- The backend logic is implemented and covered by automated tests.
- The current repository still uses in-memory repository implementations for development and testing.
- There is not yet a real database layer, so data does not persist after the program exits.
- There is not yet a GUI, so this is not a finished user-facing desktop app.

## Team boundaries
- Backend owns service logic, validation, session state, encryption rules, MFA flow, admin rules, and audit logging.
- Database implementation should provide concrete implementations of `UserRepository`, `VaultRepository`, and `AuditRepository`.
- GUI implementation should call the backend services and display the returned results or errors to the user.

## Build and test
```bash
cmake -S . -B build
cmake --build build
./build/password_manager_backend_tests
ctest --test-dir build --output-on-failure
```
