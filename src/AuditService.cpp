#include "password_manager_backend/AuditService.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>

namespace password_manager_backend {

AuditService::AuditService(AuditRepository& auditRepository) : auditRepository_(auditRepository) {}

void AuditService::recordEvent(
    const std::string& userId,
    const std::string& username,
    const std::string& action,
    const std::string& details) {
    auditRepository_.save({
        .id = createIdentifier(),
        .userId = userId,
        .username = username,
        .action = action,
        .details = details,
        .timestamp = currentTimestamp(),
    });
}

std::vector<AuditEvent> AuditService::getAllEvents() const {
    return auditRepository_.listAll();
}

std::vector<AuditEvent> AuditService::getEventsForUser(const std::string& userId) const {
    return auditRepository_.listByUserId(userId);
}

std::string AuditService::createIdentifier() {
    static constexpr char alphabet[] = "0123456789abcdef";
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<std::size_t> distribution(0, sizeof(alphabet) - 2);

    std::string identifier(24, '0');
    for (char& character : identifier) {
        character = alphabet[distribution(generator)];
    }
    return identifier;
}

std::string AuditService::currentTimestamp() {
    // ISO-like timestamps make the audit trail easier to inspect and sort when a GUI is added later.
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm timeParts{};

#if defined(_WIN32)
    // Windows uses the secure localtime_s variant instead of localtime_r.
    localtime_s(&timeParts, &nowTime);
#else
    localtime_r(&nowTime, &timeParts);
#endif

    std::ostringstream stream;
    stream << std::put_time(&timeParts, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

}  // namespace password_manager_backend
