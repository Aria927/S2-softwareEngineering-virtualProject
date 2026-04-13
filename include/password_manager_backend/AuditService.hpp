#pragma once

#include "password_manager_backend/Repositories.hpp"

namespace password_manager_backend {

// Centralises audit event creation so auth, admin, and vault services log in a consistent format.
class AuditService {
public:
    explicit AuditService(AuditRepository& auditRepository);

    void recordEvent(
        const std::string& userId,
        const std::string& username,
        const std::string& action,
        const std::string& details);

    std::vector<AuditEvent> getAllEvents() const;
    std::vector<AuditEvent> getEventsForUser(const std::string& userId) const;

private:
    static std::string createIdentifier();
    static std::string currentTimestamp();

    AuditRepository& auditRepository_;
};

}  // namespace password_manager_backend
