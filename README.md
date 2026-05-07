# Password Manager App

This repository contains a C++ desktop password manager built with openFrameworks. The app allows users to create an account, log in, add password entries, edit/delete entries, search the vault, reveal hidden passwords, and store general notes alongside each entry.

## Finished prototype build

The easiest way to test the finished prototype is the standalone executable:

[`dist/PasswordManager-SingleExe-Final/Password Manager Standalone.exe`](dist/PasswordManager-SingleExe-Final/Password%20Manager%20Standalone.exe)

This version is packaged as a single executable and runs in local in-memory mode. It does not require MySQL or any extra setup. Because it is in-memory, accounts and entries created in this version only last while the app is open.

If Windows shows an unknown app warning, choose **More info** and then **Run anyway**.

## Team test build

A folder-based team test package is also available:

[`dist/PasswordManager-TeamTest`](dist/PasswordManager-TeamTest)

This version includes launch scripts for local testing and MySQL-backed testing.

- Use `Launch-Local.cmd` for quick testing without a database.
- Use `Launch-MySQL.cmd` if you want to test persistent storage through MySQL.

## Current features

- User registration and login/logout.
- Password vault with add, edit, delete, view, and search functionality.
- Hidden passwords use a fixed number of stars so password length is not revealed.
- General notes/text can be stored on each vault entry.
- Keyboard support for tabbing through fields and pressing enter to submit forms.
- MySQL-backed storage is available for persistent testing.
- Local in-memory mode is available for quick testing without database setup.

## Security approach

The backend protects account and master passwords using salted hashes rather than storing them as plain text. Stored vault passwords are encrypted before being saved, and are only decrypted after the user has successfully logged in and unlocked the vault.

The app is designed with separate service classes for authentication, vault management, encryption, search, MFA/admin features, and audit logging. This keeps the main application logic separate from both the GUI and database implementation.

## MySQL mode

The app can run without MySQL, but persistent database testing requires these environment variables before launching:

```powershell
$env:PM_STORAGE = "mysql"
$env:PM_DB_HOST = "127.0.0.1"
$env:PM_DB_PORT = "3306"
$env:PM_DB_NAME = "passwordManager"
$env:PM_DB_USER = "root"
$env:PM_DB_PASSWORD = "your-password-here"
```

When MySQL mode is enabled, the app connects to the configured server and uses the integrated repository layer for users, vault entries, tags, and audit events.

## Development build and tests

The backend can be built and tested from the repository root:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The openFrameworks GUI project is located in:

```text
Front-end GUI/
```
