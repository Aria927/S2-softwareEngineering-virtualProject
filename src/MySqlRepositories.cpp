#include "password_manager_backend/MySqlRepositories.hpp"

#include <mysql.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace password_manager_backend {
namespace {

std::optional<std::string> getEnvironmentVariable(const char* name) {
    char* rawValue = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&rawValue, &length, name) != 0 || rawValue == nullptr || length == 0) {
        return std::nullopt;
    }

    std::string value(rawValue);
    std::free(rawValue);

    if (value.empty()) {
        return std::nullopt;
    }

    return value;
}

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::string hexEncode(const std::string& value) {
    static constexpr char alphabet[] = "0123456789abcdef";
    std::string encoded;
    encoded.reserve(value.size() * 2);

    for (unsigned char character : value) {
        encoded.push_back(alphabet[(character >> 4U) & 0x0F]);
        encoded.push_back(alphabet[character & 0x0F]);
    }

    return encoded;
}

int decodeHexNibble(char character) {
    if (character >= '0' && character <= '9') {
        return character - '0';
    }

    if (character >= 'a' && character <= 'f') {
        return character - 'a' + 10;
    }

    if (character >= 'A' && character <= 'F') {
        return character - 'A' + 10;
    }

    throw std::runtime_error("Encountered invalid hexadecimal data while decoding a database field.");
}

std::string hexDecode(const std::optional<std::string>& value) {
    if (!value.has_value() || value->empty()) {
        return {};
    }

    if (value->size() % 2 != 0) {
        throw std::runtime_error("Encountered malformed hexadecimal data while decoding a database field.");
    }

    std::string decoded;
    decoded.reserve(value->size() / 2);

    for (std::size_t index = 0; index < value->size(); index += 2) {
        const int high = decodeHexNibble((*value)[index]);
        const int low = decodeHexNibble((*value)[index + 1]);
        decoded.push_back(static_cast<char>((high << 4) | low));
    }

    return decoded;
}

std::string sqlQuote(MySqlDatabase& database, const std::string& value) {
    return "'" + database.escape(value) + "'";
}

std::string sqlHexBlob(const std::string& value) {
    return "'" + hexEncode(value) + "'";
}

std::string sqlOptionalHexBlob(const std::optional<std::string>& value) {
    if (!value.has_value()) {
        return "NULL";
    }

    return sqlHexBlob(*value);
}

std::optional<std::string> optionalCell(
    const std::unordered_map<std::string, std::optional<std::string>>& row,
    const std::string& column) {
    const auto iterator = row.find(column);
    if (iterator == row.end()) {
        throw std::runtime_error("The database result did not contain the expected column: " + column);
    }

    return iterator->second;
}

std::string requiredText(
    const std::unordered_map<std::string, std::optional<std::string>>& row,
    const std::string& column) {
    const auto value = optionalCell(row, column);
    if (!value.has_value()) {
        throw std::runtime_error("The database field was unexpectedly NULL: " + column);
    }

    return *value;
}

bool boolFromCell(const std::optional<std::string>& value) {
    return value.has_value() && *value != "0";
}

std::unordered_map<std::string, std::optional<std::string>> rowToMap(
    const MySqlQueryResult& result,
    const std::vector<std::optional<std::string>>& rowValues) {
    std::unordered_map<std::string, std::optional<std::string>> row;
    row.reserve(result.columns.size());

    for (std::size_t index = 0; index < result.columns.size(); ++index) {
        row.emplace(result.columns[index], rowValues[index]);
    }

    return row;
}

std::optional<std::uint64_t> findLocalUserId(MySqlDatabase& database, const std::string& appUserId) {
    const auto result = database.query(
        "SELECT local_id FROM users WHERE app_user_id = " + sqlQuote(database, appUserId) + " LIMIT 1");

    if (result.rows.empty()) {
        return std::nullopt;
    }

    const auto row = rowToMap(result, result.rows.front());
    return static_cast<std::uint64_t>(std::stoull(requiredText(row, "local_id")));
}

std::optional<std::uint64_t> findLocalEntryId(MySqlDatabase& database, const std::string& appEntryId) {
    const auto result = database.query(
        "SELECT local_id FROM entries WHERE app_entry_id = " + sqlQuote(database, appEntryId) + " LIMIT 1");

    if (result.rows.empty()) {
        return std::nullopt;
    }

    const auto row = rowToMap(result, result.rows.front());
    return static_cast<std::uint64_t>(std::stoull(requiredText(row, "local_id")));
}

void replaceTags(MySqlDatabase& database, std::uint64_t localEntryId, const std::vector<std::string>& tags) {
    database.execute("DELETE FROM entry_tags WHERE entry_local_id = " + std::to_string(localEntryId));

    for (const auto& tag : tags) {
        database.execute(
            "INSERT INTO tags (name) VALUES (" + sqlQuote(database, tag) +
            ") ON DUPLICATE KEY UPDATE name = VALUES(name)");

        const auto tagResult = database.query(
            "SELECT tag_id FROM tags WHERE name = " + sqlQuote(database, tag) + " LIMIT 1");

        if (tagResult.rows.empty()) {
            throw std::runtime_error("A tag insert completed without returning a tag identifier.");
        }

        const auto tagRow = rowToMap(tagResult, tagResult.rows.front());
        const auto tagId = requiredText(tagRow, "tag_id");

        database.execute(
            "INSERT INTO entry_tags (entry_local_id, tag_id) VALUES (" + std::to_string(localEntryId) + ", " + tagId +
            ") ON DUPLICATE KEY UPDATE tag_id = VALUES(tag_id)");
    }
}

std::vector<std::string> loadTags(MySqlDatabase& database, std::uint64_t localEntryId) {
    const auto result = database.query(
        "SELECT t.name "
        "FROM tags t "
        "INNER JOIN entry_tags et ON et.tag_id = t.tag_id "
        "WHERE et.entry_local_id = " +
        std::to_string(localEntryId) + " ORDER BY t.name");

    std::vector<std::string> tags;
    tags.reserve(result.rows.size());

    for (const auto& rowValues : result.rows) {
        const auto row = rowToMap(result, rowValues);
        tags.push_back(requiredText(row, "name"));
    }

    return tags;
}

UserRole roleFromDatabase(const std::string& value) {
    return toLower(value) == "admin" ? UserRole::Admin : UserRole::Standard;
}

std::string roleToDatabase(UserRole role) {
    return role == UserRole::Admin ? "admin" : "standard";
}

UserAccount mapUserRow(const MySqlQueryResult& result, const std::vector<std::optional<std::string>>& rowValues) {
    const auto row = rowToMap(result, rowValues);

    UserAccount user{
        .id = requiredText(row, "app_user_id"),
        .username = requiredText(row, "username"),
        .email = requiredText(row, "email"),
        .passwordHash = hexDecode(optionalCell(row, "password_hash")),
        .passwordSalt = hexDecode(optionalCell(row, "password_salt")),
        .masterPasswordHash = hexDecode(optionalCell(row, "master_password_hash")),
        .masterPasswordSalt = hexDecode(optionalCell(row, "master_password_salt")),
        .role = roleFromDatabase(requiredText(row, "role")),
        .mfaEnabled = boolFromCell(optionalCell(row, "mfa_enabled")),
        .encryptedMfaSecret = std::nullopt,
        .masterWrappedVaultKey = {
            hexDecode(optionalCell(row, "master_wrapped_vault_key")),
            hexDecode(optionalCell(row, "master_wrapped_vault_key_salt")),
            hexDecode(optionalCell(row, "master_wrapped_vault_key_iv")),
        },
        .passwordWrappedVaultKey = {
            hexDecode(optionalCell(row, "password_wrapped_vault_key")),
            hexDecode(optionalCell(row, "password_wrapped_vault_key_salt")),
            hexDecode(optionalCell(row, "password_wrapped_vault_key_iv")),
        },
        .recoveryWrappedVaultKey = {
            hexDecode(optionalCell(row, "recovery_wrapped_vault_key")),
            hexDecode(optionalCell(row, "recovery_wrapped_vault_key_salt")),
            hexDecode(optionalCell(row, "recovery_wrapped_vault_key_iv")),
        },
    };

    const auto encryptedMfaCipherText = optionalCell(row, "mfa_ciphertext");
    if (encryptedMfaCipherText.has_value()) {
        user.encryptedMfaSecret = EncryptedBlob{
            hexDecode(encryptedMfaCipherText),
            hexDecode(optionalCell(row, "mfa_salt")),
            hexDecode(optionalCell(row, "mfa_iv")),
        };
    }

    return user;
}

AuditEvent mapAuditRow(const MySqlQueryResult& result, const std::vector<std::optional<std::string>>& rowValues) {
    const auto row = rowToMap(result, rowValues);

    return AuditEvent{
        .id = requiredText(row, "app_audit_id"),
        .userId = optionalCell(row, "user_app_id").value_or(""),
        .username = requiredText(row, "username"),
        .action = requiredText(row, "action"),
        .details = requiredText(row, "details"),
        .timestamp = requiredText(row, "timestamp"),
    };
}

VaultEntry mapEntryRow(
    MySqlDatabase& database,
    const MySqlQueryResult& result,
    const std::vector<std::optional<std::string>>& rowValues) {
    const auto row = rowToMap(result, rowValues);
    const auto localId = static_cast<std::uint64_t>(std::stoull(requiredText(row, "local_id")));

    return VaultEntry{
        .id = requiredText(row, "app_entry_id"),
        .userId = requiredText(row, "app_user_id"),
        .title = requiredText(row, "title"),
        .site = requiredText(row, "site"),
        .username = requiredText(row, "entry_username"),
        .encryptedPassword = hexDecode(optionalCell(row, "encrypted_password")),
        .encryptionSalt = hexDecode(optionalCell(row, "encryption_salt")),
        .encryptionIv = hexDecode(optionalCell(row, "encryption_iv")),
        .notes = requiredText(row, "notes"),
        .tags = loadTags(database, localId),
    };
}

std::string userSelectSql() {
    return
        "SELECT app_user_id, username, email, password_hash, password_salt, master_password_hash, "
        "master_password_salt, role, mfa_enabled, mfa_ciphertext, mfa_salt, mfa_iv, "
        "master_wrapped_vault_key, master_wrapped_vault_key_salt, master_wrapped_vault_key_iv, "
        "password_wrapped_vault_key, password_wrapped_vault_key_salt, password_wrapped_vault_key_iv, "
        "recovery_wrapped_vault_key, recovery_wrapped_vault_key_salt, recovery_wrapped_vault_key_iv "
        "FROM users";
}

std::string entrySelectSql() {
    return
        "SELECT e.local_id, e.app_entry_id, u.app_user_id, e.title, e.site, e.entry_username, "
        "e.encrypted_password, e.encryption_salt, e.encryption_iv, e.notes "
        "FROM entries e "
        "INNER JOIN users u ON u.local_id = e.user_local_id";
}

}  // namespace

struct MySqlDatabase::Impl {
    MYSQL* connection{nullptr};
    MySqlConnectionConfig config;
};

bool MySqlConnectionConfig::isEnabledFromEnvironment() {
    const auto storageMode = getEnvironmentVariable("PM_STORAGE");
    return storageMode.has_value() && toLower(*storageMode) == "mysql";
}

MySqlConnectionConfig MySqlConnectionConfig::fromEnvironment() {
    MySqlConnectionConfig config;

    if (const auto value = getEnvironmentVariable("PM_DB_HOST"); value.has_value()) {
        config.host = *value;
    }

    if (const auto value = getEnvironmentVariable("PM_DB_PORT"); value.has_value()) {
        config.port = static_cast<unsigned int>(std::stoul(*value));
    }

    if (const auto value = getEnvironmentVariable("PM_DB_USER"); value.has_value()) {
        config.username = *value;
    }

    if (const auto value = getEnvironmentVariable("PM_DB_PASSWORD"); value.has_value()) {
        config.password = *value;
    }

    if (const auto value = getEnvironmentVariable("PM_DB_NAME"); value.has_value()) {
        config.database = *value;
    }

    return config;
}

MySqlDatabase::MySqlDatabase(MySqlConnectionConfig config) : impl_(std::make_unique<Impl>()) {
    impl_->config = std::move(config);
    impl_->connection = mysql_init(nullptr);
    if (impl_->connection == nullptr) {
        throw std::runtime_error("MySQL initialisation failed.");
    }

    if (mysql_real_connect(
            impl_->connection,
            impl_->config.host.c_str(),
            impl_->config.username.c_str(),
            impl_->config.password.c_str(),
            nullptr,
            impl_->config.port,
            nullptr,
            0) == nullptr) {
        const std::string error = mysql_error(impl_->connection);
        mysql_close(impl_->connection);
        impl_->connection = nullptr;
        throw std::runtime_error("MySQL connection failed: " + error);
    }

    execute("SET NAMES utf8mb4");
    execute("CREATE DATABASE IF NOT EXISTS `" + impl_->config.database + "`");

    if (mysql_select_db(impl_->connection, impl_->config.database.c_str()) != 0) {
        throw std::runtime_error("Selecting the MySQL database failed: " + std::string(mysql_error(impl_->connection)));
    }

    // The schema keeps the teammate's MySQL direction but extends it to match the backend models already in use.
    execute(
        "CREATE TABLE IF NOT EXISTS users ("
        "local_id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "app_user_id VARCHAR(24) NOT NULL UNIQUE, "
        "username VARCHAR(255) NOT NULL UNIQUE, "
        "email VARCHAR(255) NOT NULL UNIQUE, "
        "password_hash LONGTEXT NOT NULL, "
        "password_salt TEXT NOT NULL, "
        "master_password_hash LONGTEXT NOT NULL, "
        "master_password_salt TEXT NOT NULL, "
        "role VARCHAR(20) NOT NULL, "
        "mfa_enabled TINYINT(1) NOT NULL DEFAULT 0, "
        "mfa_ciphertext LONGTEXT NULL, "
        "mfa_salt TEXT NULL, "
        "mfa_iv TEXT NULL, "
        "master_wrapped_vault_key LONGTEXT NOT NULL, "
        "master_wrapped_vault_key_salt TEXT NOT NULL, "
        "master_wrapped_vault_key_iv TEXT NOT NULL, "
        "password_wrapped_vault_key LONGTEXT NOT NULL, "
        "password_wrapped_vault_key_salt TEXT NOT NULL, "
        "password_wrapped_vault_key_iv TEXT NOT NULL, "
        "recovery_wrapped_vault_key LONGTEXT NOT NULL, "
        "recovery_wrapped_vault_key_salt TEXT NOT NULL, "
        "recovery_wrapped_vault_key_iv TEXT NOT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")");

    execute(
        "CREATE TABLE IF NOT EXISTS entries ("
        "local_id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "app_entry_id VARCHAR(24) NOT NULL UNIQUE, "
        "user_local_id BIGINT UNSIGNED NOT NULL, "
        "title VARCHAR(255) NOT NULL, "
        "site VARCHAR(255) NOT NULL, "
        "entry_username VARCHAR(255) NOT NULL, "
        "encrypted_password LONGTEXT NOT NULL, "
        "encryption_salt TEXT NOT NULL, "
        "encryption_iv TEXT NOT NULL, "
        "notes TEXT NOT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
        "FOREIGN KEY (user_local_id) REFERENCES users(local_id) ON DELETE CASCADE"
        ")");

    execute(
        "CREATE TABLE IF NOT EXISTS tags ("
        "tag_id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "name VARCHAR(50) NOT NULL UNIQUE"
        ")");

    execute(
        "CREATE TABLE IF NOT EXISTS entry_tags ("
        "entry_local_id BIGINT UNSIGNED NOT NULL, "
        "tag_id BIGINT UNSIGNED NOT NULL, "
        "PRIMARY KEY (entry_local_id, tag_id), "
        "FOREIGN KEY (entry_local_id) REFERENCES entries(local_id) ON DELETE CASCADE, "
        "FOREIGN KEY (tag_id) REFERENCES tags(tag_id) ON DELETE CASCADE"
        ")");

    execute(
        "CREATE TABLE IF NOT EXISTS audit_events ("
        "local_id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "app_audit_id VARCHAR(24) NOT NULL UNIQUE, "
        "user_app_id VARCHAR(24) NULL, "
        "username VARCHAR(255) NOT NULL, "
        "action VARCHAR(255) NOT NULL, "
        "details TEXT NOT NULL, "
        "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")");
}

MySqlDatabase::~MySqlDatabase() {
    if (impl_ != nullptr && impl_->connection != nullptr) {
        mysql_close(impl_->connection);
    }
}

std::string MySqlDatabase::escape(const std::string& value) const {
    std::string escaped(value.size() * 2 + 1, '\0');
    const auto escapedLength = mysql_real_escape_string(
        impl_->connection,
        escaped.data(),
        value.c_str(),
        static_cast<unsigned long>(value.size()));
    escaped.resize(escapedLength);
    return escaped;
}

void MySqlDatabase::execute(const std::string& sql) const {
    if (mysql_query(impl_->connection, sql.c_str()) != 0) {
        throw std::runtime_error("MySQL statement failed: " + std::string(mysql_error(impl_->connection)));
    }

    MYSQL_RES* result = mysql_store_result(impl_->connection);
    if (result != nullptr) {
        mysql_free_result(result);
    }
}

MySqlQueryResult MySqlDatabase::query(const std::string& sql) const {
    if (mysql_query(impl_->connection, sql.c_str()) != 0) {
        throw std::runtime_error("MySQL query failed: " + std::string(mysql_error(impl_->connection)));
    }

    MYSQL_RES* rawResult = mysql_store_result(impl_->connection);
    if (rawResult == nullptr) {
        return {};
    }

    MySqlQueryResult result;
    const unsigned int fieldCount = mysql_num_fields(rawResult);
    result.columns.reserve(fieldCount);

    MYSQL_FIELD* fields = mysql_fetch_fields(rawResult);
    for (unsigned int index = 0; index < fieldCount; ++index) {
        result.columns.emplace_back(fields[index].name);
    }

    MYSQL_ROW row = nullptr;
    while ((row = mysql_fetch_row(rawResult)) != nullptr) {
        const unsigned long* lengths = mysql_fetch_lengths(rawResult);
        std::vector<std::optional<std::string>> values;
        values.reserve(fieldCount);

        for (unsigned int index = 0; index < fieldCount; ++index) {
            if (row[index] == nullptr) {
                values.push_back(std::nullopt);
            } else {
                values.emplace_back(std::string(row[index], lengths[index]));
            }
        }

        result.rows.push_back(std::move(values));
    }

    mysql_free_result(rawResult);
    return result;
}

std::uint64_t MySqlDatabase::lastInsertId() const {
    return mysql_insert_id(impl_->connection);
}

MySqlUserRepository::MySqlUserRepository(std::shared_ptr<MySqlDatabase> database) : database_(std::move(database)) {}

std::optional<UserAccount> MySqlUserRepository::findByUsername(const std::string& username) const {
    const auto result = database_->query(
        userSelectSql() + " WHERE username = " + sqlQuote(*database_, username) + " LIMIT 1");

    if (result.rows.empty()) {
        return std::nullopt;
    }

    return mapUserRow(result, result.rows.front());
}

std::optional<UserAccount> MySqlUserRepository::findByEmail(const std::string& email) const {
    const auto result =
        database_->query(userSelectSql() + " WHERE email = " + sqlQuote(*database_, email) + " LIMIT 1");

    if (result.rows.empty()) {
        return std::nullopt;
    }

    return mapUserRow(result, result.rows.front());
}

std::optional<UserAccount> MySqlUserRepository::findById(const std::string& id) const {
    const auto result =
        database_->query(userSelectSql() + " WHERE app_user_id = " + sqlQuote(*database_, id) + " LIMIT 1");

    if (result.rows.empty()) {
        return std::nullopt;
    }

    return mapUserRow(result, result.rows.front());
}

std::vector<UserAccount> MySqlUserRepository::listAll() const {
    const auto result = database_->query(userSelectSql() + " ORDER BY username");

    std::vector<UserAccount> users;
    users.reserve(result.rows.size());
    for (const auto& rowValues : result.rows) {
        users.push_back(mapUserRow(result, rowValues));
    }

    return users;
}

void MySqlUserRepository::save(const UserAccount& user) {
    std::ostringstream sql;
    sql << "INSERT INTO users ("
        << "app_user_id, username, email, password_hash, password_salt, master_password_hash, "
        << "master_password_salt, role, mfa_enabled, mfa_ciphertext, mfa_salt, mfa_iv, "
        << "master_wrapped_vault_key, master_wrapped_vault_key_salt, master_wrapped_vault_key_iv, "
        << "password_wrapped_vault_key, password_wrapped_vault_key_salt, password_wrapped_vault_key_iv, "
        << "recovery_wrapped_vault_key, recovery_wrapped_vault_key_salt, recovery_wrapped_vault_key_iv"
        << ") VALUES ("
        << sqlQuote(*database_, user.id) << ", "
        << sqlQuote(*database_, user.username) << ", "
        << sqlQuote(*database_, user.email) << ", "
        << sqlHexBlob(user.passwordHash) << ", "
        << sqlHexBlob(user.passwordSalt) << ", "
        << sqlHexBlob(user.masterPasswordHash) << ", "
        << sqlHexBlob(user.masterPasswordSalt) << ", "
        << sqlQuote(*database_, roleToDatabase(user.role)) << ", "
        << (user.mfaEnabled ? "1" : "0") << ", "
        << sqlOptionalHexBlob(user.encryptedMfaSecret.has_value() ? std::optional(user.encryptedMfaSecret->cipherText) : std::nullopt)
        << ", "
        << sqlOptionalHexBlob(user.encryptedMfaSecret.has_value() ? std::optional(user.encryptedMfaSecret->salt) : std::nullopt)
        << ", "
        << sqlOptionalHexBlob(user.encryptedMfaSecret.has_value() ? std::optional(user.encryptedMfaSecret->iv) : std::nullopt)
        << ", "
        << sqlHexBlob(user.masterWrappedVaultKey.cipherText) << ", "
        << sqlHexBlob(user.masterWrappedVaultKey.salt) << ", "
        << sqlHexBlob(user.masterWrappedVaultKey.iv) << ", "
        << sqlHexBlob(user.passwordWrappedVaultKey.cipherText) << ", "
        << sqlHexBlob(user.passwordWrappedVaultKey.salt) << ", "
        << sqlHexBlob(user.passwordWrappedVaultKey.iv) << ", "
        << sqlHexBlob(user.recoveryWrappedVaultKey.cipherText) << ", "
        << sqlHexBlob(user.recoveryWrappedVaultKey.salt) << ", "
        << sqlHexBlob(user.recoveryWrappedVaultKey.iv) << ")";

    database_->execute(sql.str());
}

void MySqlUserRepository::update(const UserAccount& user) {
    std::ostringstream sql;
    sql << "UPDATE users SET "
        << "username = " << sqlQuote(*database_, user.username) << ", "
        << "email = " << sqlQuote(*database_, user.email) << ", "
        << "password_hash = " << sqlHexBlob(user.passwordHash) << ", "
        << "password_salt = " << sqlHexBlob(user.passwordSalt) << ", "
        << "master_password_hash = " << sqlHexBlob(user.masterPasswordHash) << ", "
        << "master_password_salt = " << sqlHexBlob(user.masterPasswordSalt) << ", "
        << "role = " << sqlQuote(*database_, roleToDatabase(user.role)) << ", "
        << "mfa_enabled = " << (user.mfaEnabled ? "1" : "0") << ", "
        << "mfa_ciphertext = "
        << sqlOptionalHexBlob(user.encryptedMfaSecret.has_value() ? std::optional(user.encryptedMfaSecret->cipherText) : std::nullopt)
        << ", "
        << "mfa_salt = "
        << sqlOptionalHexBlob(user.encryptedMfaSecret.has_value() ? std::optional(user.encryptedMfaSecret->salt) : std::nullopt)
        << ", "
        << "mfa_iv = "
        << sqlOptionalHexBlob(user.encryptedMfaSecret.has_value() ? std::optional(user.encryptedMfaSecret->iv) : std::nullopt)
        << ", "
        << "master_wrapped_vault_key = " << sqlHexBlob(user.masterWrappedVaultKey.cipherText) << ", "
        << "master_wrapped_vault_key_salt = " << sqlHexBlob(user.masterWrappedVaultKey.salt) << ", "
        << "master_wrapped_vault_key_iv = " << sqlHexBlob(user.masterWrappedVaultKey.iv) << ", "
        << "password_wrapped_vault_key = " << sqlHexBlob(user.passwordWrappedVaultKey.cipherText) << ", "
        << "password_wrapped_vault_key_salt = " << sqlHexBlob(user.passwordWrappedVaultKey.salt) << ", "
        << "password_wrapped_vault_key_iv = " << sqlHexBlob(user.passwordWrappedVaultKey.iv) << ", "
        << "recovery_wrapped_vault_key = " << sqlHexBlob(user.recoveryWrappedVaultKey.cipherText) << ", "
        << "recovery_wrapped_vault_key_salt = " << sqlHexBlob(user.recoveryWrappedVaultKey.salt) << ", "
        << "recovery_wrapped_vault_key_iv = " << sqlHexBlob(user.recoveryWrappedVaultKey.iv) << " "
        << "WHERE app_user_id = " << sqlQuote(*database_, user.id);

    database_->execute(sql.str());
}

MySqlVaultRepository::MySqlVaultRepository(std::shared_ptr<MySqlDatabase> database) : database_(std::move(database)) {}

void MySqlVaultRepository::save(const VaultEntry& entry) {
    const auto localUserId = findLocalUserId(*database_, entry.userId);
    if (!localUserId.has_value()) {
        throw std::runtime_error("The user for a vault entry could not be found in MySQL storage.");
    }

    std::ostringstream sql;
    sql << "INSERT INTO entries ("
        << "app_entry_id, user_local_id, title, site, entry_username, encrypted_password, encryption_salt, "
        << "encryption_iv, notes"
        << ") VALUES ("
        << sqlQuote(*database_, entry.id) << ", "
        << *localUserId << ", "
        << sqlQuote(*database_, entry.title) << ", "
        << sqlQuote(*database_, entry.site) << ", "
        << sqlQuote(*database_, entry.username) << ", "
        << sqlHexBlob(entry.encryptedPassword) << ", "
        << sqlHexBlob(entry.encryptionSalt) << ", "
        << sqlHexBlob(entry.encryptionIv) << ", "
        << sqlQuote(*database_, entry.notes) << ")";

    database_->execute(sql.str());
    replaceTags(*database_, database_->lastInsertId(), entry.tags);
}

std::optional<VaultEntry> MySqlVaultRepository::findById(const std::string& entryId) const {
    const auto result =
        database_->query(entrySelectSql() + " WHERE e.app_entry_id = " + sqlQuote(*database_, entryId) + " LIMIT 1");

    if (result.rows.empty()) {
        return std::nullopt;
    }

    return mapEntryRow(*database_, result, result.rows.front());
}

std::vector<VaultEntry> MySqlVaultRepository::findByUserId(const std::string& userId) const {
    const auto result =
        database_->query(entrySelectSql() + " WHERE u.app_user_id = " + sqlQuote(*database_, userId) + " ORDER BY e.title");

    std::vector<VaultEntry> entries;
    entries.reserve(result.rows.size());
    for (const auto& rowValues : result.rows) {
        entries.push_back(mapEntryRow(*database_, result, rowValues));
    }

    return entries;
}

void MySqlVaultRepository::update(const VaultEntry& entry) {
    const auto localEntryId = findLocalEntryId(*database_, entry.id);
    if (!localEntryId.has_value()) {
        throw std::runtime_error("The vault entry could not be found in MySQL storage.");
    }

    std::ostringstream sql;
    sql << "UPDATE entries SET "
        << "title = " << sqlQuote(*database_, entry.title) << ", "
        << "site = " << sqlQuote(*database_, entry.site) << ", "
        << "entry_username = " << sqlQuote(*database_, entry.username) << ", "
        << "encrypted_password = " << sqlHexBlob(entry.encryptedPassword) << ", "
        << "encryption_salt = " << sqlHexBlob(entry.encryptionSalt) << ", "
        << "encryption_iv = " << sqlHexBlob(entry.encryptionIv) << ", "
        << "notes = " << sqlQuote(*database_, entry.notes) << " "
        << "WHERE app_entry_id = " << sqlQuote(*database_, entry.id);

    database_->execute(sql.str());
    replaceTags(*database_, *localEntryId, entry.tags);
}

bool MySqlVaultRepository::remove(const std::string& entryId, const std::string& userId) {
    const auto localUserId = findLocalUserId(*database_, userId);
    if (!localUserId.has_value()) {
        return false;
    }

    const auto entryLookup = database_->query(
        "SELECT local_id FROM entries WHERE app_entry_id = " + sqlQuote(*database_, entryId) +
        " AND user_local_id = " + std::to_string(*localUserId) + " LIMIT 1");

    if (entryLookup.rows.empty()) {
        return false;
    }

    database_->execute("DELETE FROM entries WHERE app_entry_id = " + sqlQuote(*database_, entryId));
    return true;
}

MySqlAuditRepository::MySqlAuditRepository(std::shared_ptr<MySqlDatabase> database) : database_(std::move(database)) {}

void MySqlAuditRepository::save(const AuditEvent& event) {
    std::ostringstream sql;
    sql << "INSERT INTO audit_events (app_audit_id, user_app_id, username, action, details) VALUES ("
        << sqlQuote(*database_, event.id) << ", ";

    if (event.userId.empty()) {
        sql << "NULL";
    } else {
        sql << sqlQuote(*database_, event.userId);
    }

    sql << ", "
        << sqlQuote(*database_, event.username) << ", "
        << sqlQuote(*database_, event.action) << ", "
        << sqlQuote(*database_, event.details) << ")";

    database_->execute(sql.str());
}

std::vector<AuditEvent> MySqlAuditRepository::listAll() const {
    const auto result = database_->query(
        "SELECT app_audit_id, user_app_id, username, action, details, "
        "DATE_FORMAT(timestamp, '%Y-%m-%d %H:%i:%s') AS timestamp "
        "FROM audit_events ORDER BY timestamp DESC");

    std::vector<AuditEvent> events;
    events.reserve(result.rows.size());
    for (const auto& rowValues : result.rows) {
        events.push_back(mapAuditRow(result, rowValues));
    }

    return events;
}

std::vector<AuditEvent> MySqlAuditRepository::listByUserId(const std::string& userId) const {
    const auto result = database_->query(
        "SELECT app_audit_id, user_app_id, username, action, details, "
        "DATE_FORMAT(timestamp, '%Y-%m-%d %H:%i:%s') AS timestamp "
        "FROM audit_events WHERE user_app_id = " +
        sqlQuote(*database_, userId) + " ORDER BY timestamp DESC");

    std::vector<AuditEvent> events;
    events.reserve(result.rows.size());
    for (const auto& rowValues : result.rows) {
        events.push_back(mapAuditRow(result, rowValues));
    }

    return events;
}

}  // namespace password_manager_backend
