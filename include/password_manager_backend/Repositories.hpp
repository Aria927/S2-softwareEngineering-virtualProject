#pragma once

#include "password_manager_backend/Models.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace password_manager_backend {

// Backend-facing contract for user storage. The database teammate can replace this implementation.
class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::optional<UserAccount> findByUsername(const std::string& username) const = 0;
    virtual std::optional<UserAccount> findByEmail(const std::string& email) const = 0;
    virtual std::optional<UserAccount> findById(const std::string& id) const = 0;
    virtual std::vector<UserAccount> listAll() const = 0;
    virtual void save(const UserAccount& user) = 0;
    virtual void update(const UserAccount& user) = 0;
};

// Backend-facing contract for vault entry persistence.
class VaultRepository {
public:
    virtual ~VaultRepository() = default;

    virtual void save(const VaultEntry& entry) = 0;
    virtual std::optional<VaultEntry> findById(const std::string& entryId) const = 0;
    virtual std::vector<VaultEntry> findByUserId(const std::string& userId) const = 0;
    virtual void update(const VaultEntry& entry) = 0;
    virtual bool remove(const std::string& entryId, const std::string& userId) = 0;
};

// Audit storage is separate so logs can be persisted without mixing them into user or vault repositories.
class AuditRepository {
public:
    virtual ~AuditRepository() = default;

    virtual void save(const AuditEvent& event) = 0;
    virtual std::vector<AuditEvent> listAll() const = 0;
    virtual std::vector<AuditEvent> listByUserId(const std::string& userId) const = 0;
};

// In-memory repository used for local testing before a real database is connected.
class InMemoryUserRepository final : public UserRepository {
public:
    std::optional<UserAccount> findByUsername(const std::string& username) const override {
        auto iterator = usersByUsername_.find(username);
        if (iterator == usersByUsername_.end()) {
            return std::nullopt;
        }
        return iterator->second;
    }

    std::optional<UserAccount> findByEmail(const std::string& email) const override {
        auto iterator = emailToUsername_.find(email);
        if (iterator == emailToUsername_.end()) {
            return std::nullopt;
        }
        return findByUsername(iterator->second);
    }

    std::optional<UserAccount> findById(const std::string& id) const override {
        auto iterator = usersById_.find(id);
        if (iterator == usersById_.end()) {
            return std::nullopt;
        }
        return iterator->second;
    }

    std::vector<UserAccount> listAll() const override {
        std::vector<UserAccount> users;
        users.reserve(usersById_.size());
        for (const auto& [id, user] : usersById_) {
            (void)id;
            users.push_back(user);
        }
        return users;
    }

    void save(const UserAccount& user) override {
        usersByUsername_[user.username] = user;
        usersById_[user.id] = user;
        emailToUsername_[user.email] = user.username;
    }

    void update(const UserAccount& user) override {
        save(user);
    }

private:
    std::unordered_map<std::string, UserAccount> usersByUsername_;
    std::unordered_map<std::string, UserAccount> usersById_;
    std::unordered_map<std::string, std::string> emailToUsername_;
};

// In-memory vault storage that keeps backend development separate from database design work.
class InMemoryVaultRepository final : public VaultRepository {
public:
    void save(const VaultEntry& entry) override {
        entriesById_[entry.id] = entry;
    }

    std::optional<VaultEntry> findById(const std::string& entryId) const override {
        auto iterator = entriesById_.find(entryId);
        if (iterator == entriesById_.end()) {
            return std::nullopt;
        }
        return iterator->second;
    }

    std::vector<VaultEntry> findByUserId(const std::string& userId) const override {
        std::vector<VaultEntry> entries;
        for (const auto& [id, entry] : entriesById_) {
            (void)id;
            if (entry.userId == userId) {
                entries.push_back(entry);
            }
        }

        // Sorting here gives the GUI a stable default ordering without pushing UI logic into the service layer.
        std::sort(entries.begin(), entries.end(), [](const VaultEntry& left, const VaultEntry& right) {
            return left.title < right.title;
        });

        return entries;
    }

    void update(const VaultEntry& entry) override {
        entriesById_[entry.id] = entry;
    }

    bool remove(const std::string& entryId, const std::string& userId) override {
        auto iterator = entriesById_.find(entryId);
        if (iterator == entriesById_.end() || iterator->second.userId != userId) {
            return false;
        }

        entriesById_.erase(iterator);
        return true;
    }

private:
    std::unordered_map<std::string, VaultEntry> entriesById_;
};

// In-memory audit storage keeps security logging testable before a real database exists.
class InMemoryAuditRepository final : public AuditRepository {
public:
    void save(const AuditEvent& event) override {
        events_.push_back(event);
    }

    std::vector<AuditEvent> listAll() const override {
        return events_;
    }

    std::vector<AuditEvent> listByUserId(const std::string& userId) const override {
        std::vector<AuditEvent> matches;
        for (const auto& event : events_) {
            if (event.userId == userId) {
                matches.push_back(event);
            }
        }
        return matches;
    }

private:
    std::vector<AuditEvent> events_;
};

}  // namespace password_manager_backend
