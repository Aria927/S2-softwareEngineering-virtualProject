# Password Manager Backend

This repository currently contains the backend service layer for a local desktop password manager written in C++.

## Included backend components
- `AuthService` handles registration, login, logout, and master-password verification.
- `VaultService` handles vault entry creation, listing, update, deletion, and gated access rules.
- `CryptoService` hashes account credentials with PBKDF2-HMAC-SHA256 and encrypts saved passwords with AES-256-CBC.
- `SearchService` filters decrypted vault entries by title, site, username, notes, and tags.

## Team boundaries
- Backend owns service logic, validation, session state, and encryption rules.
- Database implementation should provide concrete implementations of `UserRepository` and `VaultRepository`.
- GUI implementation should call the backend services and render the returned results or error messages.

## Build and test
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
