#pragma once

#include "password_manager_backend/Repositories.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace password_manager_backend {

struct MySqlConnectionConfig {
    std::string host{"127.0.0.1"};
    unsigned int port{3306};
    std::string username{"root"};
    std::string password;
    std::string database{"passwordManager"};

    static bool isEnabledFromEnvironment();
    static MySqlConnectionConfig fromEnvironment();
};

struct MySqlQueryResult {
    std::vector<std::string> columns;
    std::vector<std::vector<std::optional<std::string>>> rows;
};

// Keeps the raw MySQL connection details out of the service layer and owns schema setup.
class MySqlDatabase {
public:
    explicit MySqlDatabase(MySqlConnectionConfig config);
    ~MySqlDatabase();

    MySqlDatabase(const MySqlDatabase&) = delete;
    MySqlDatabase& operator=(const MySqlDatabase&) = delete;

    std::string escape(const std::string& value) const;
    void execute(const std::string& sql) const;
    MySqlQueryResult query(const std::string& sql) const;
    std::uint64_t lastInsertId() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class MySqlUserRepository final : public UserRepository {
public:
    explicit MySqlUserRepository(std::shared_ptr<MySqlDatabase> database);

    std::optional<UserAccount> findByUsername(const std::string& username) const override;
    std::optional<UserAccount> findByEmail(const std::string& email) const override;
    std::optional<UserAccount> findById(const std::string& id) const override;
    std::vector<UserAccount> listAll() const override;
    void save(const UserAccount& user) override;
    void update(const UserAccount& user) override;

private:
    std::shared_ptr<MySqlDatabase> database_;
};

class MySqlVaultRepository final : public VaultRepository {
public:
    explicit MySqlVaultRepository(std::shared_ptr<MySqlDatabase> database);

    void save(const VaultEntry& entry) override;
    std::optional<VaultEntry> findById(const std::string& entryId) const override;
    std::vector<VaultEntry> findByUserId(const std::string& userId) const override;
    void update(const VaultEntry& entry) override;
    bool remove(const std::string& entryId, const std::string& userId) override;

private:
    std::shared_ptr<MySqlDatabase> database_;
};

class MySqlAuditRepository final : public AuditRepository {
public:
    explicit MySqlAuditRepository(std::shared_ptr<MySqlDatabase> database);

    void save(const AuditEvent& event) override;
    std::vector<AuditEvent> listAll() const override;
    std::vector<AuditEvent> listByUserId(const std::string& userId) const override;

private:
    std::shared_ptr<MySqlDatabase> database_;
};

}  // namespace password_manager_backend
