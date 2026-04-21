# Password Manager Backend

This repository contains the backend service layer for a local desktop password manager written in C++. It is now also wired into the openFrameworks GUI project, and it can run either with temporary in-memory storage or with a MySQL-backed repository implementation.

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
- The GUI project is integrated with the backend service layer for login, registration, vault CRUD, search, delete, and logout.
- The app still defaults to in-memory storage unless MySQL mode is explicitly enabled.
- A MySQL-backed repository implementation is included for users, vault entries, tags, and audit events.
- The MySQL schema in [`Database/script.txt`](Database/script.txt) extends the original team database draft so it matches the backend models already used by the app.

## Team boundaries
- Backend owns service logic, validation, session state, encryption rules, MFA flow, admin rules, and audit logging.
- Database implementation should provide concrete implementations of `UserRepository`, `VaultRepository`, and `AuditRepository`.
- GUI implementation should call the backend services and display the returned results or errors to the user.

## MySQL mode
The GUI continues to work without a database by using in-memory storage. To enable MySQL persistence, set these environment variables before launching the app:

```powershell
$env:PM_STORAGE = "mysql"
$env:PM_DB_HOST = "127.0.0.1"
$env:PM_DB_PORT = "3306"
$env:PM_DB_NAME = "passwordManager"
$env:PM_DB_USER = "root"
$env:PM_DB_PASSWORD = "your-password-here"
```

When MySQL mode is enabled, the app will connect to the configured server, create the database if needed, and ensure the integrated schema exists. If MySQL mode is not enabled, the app falls back to in-memory storage.

## Build and test
```bash
cmake -S . -B build
cmake --build build
./build/password_manager_backend_tests
ctest --test-dir build --output-on-failure
```
