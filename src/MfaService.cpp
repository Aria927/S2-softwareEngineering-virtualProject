#include "password_manager_backend/MfaService.hpp"

#include <iomanip>
#include <sstream>

namespace password_manager_backend {
namespace {

constexpr std::int64_t kStepSeconds = 30;

}  // namespace

MfaService::MfaService(CryptoService& cryptoService) : cryptoService_(cryptoService) {}

Result<std::string> MfaService::createAndStoreSecret(UserAccount& user, const std::string& accountPassword) const {
    if (accountPassword.empty()) {
        return {false, "Account password is required to enable MFA.", std::nullopt};
    }

    const std::string secret = generateSecret();
    user.encryptedMfaSecret = cryptoService_.encrypt(secret, accountPassword);
    user.mfaEnabled = true;
    return {true, "MFA enabled.", secret};
}

OperationResult MfaService::clearSecret(UserAccount& user) const {
    user.mfaEnabled = false;
    user.encryptedMfaSecret.reset();
    return {true, "MFA disabled."};
}

Result<std::string> MfaService::decryptSecret(const UserAccount& user, const std::string& accountPassword) const {
    if (!user.mfaEnabled || !user.encryptedMfaSecret.has_value()) {
        return {false, "MFA is not enabled for this user.", std::nullopt};
    }

    return {
        true,
        "MFA secret loaded.",
        cryptoService_.decrypt(*user.encryptedMfaSecret, accountPassword),
    };
}

bool MfaService::verifyCode(
    const std::string& secret,
    const std::string& submittedCode,
    std::chrono::system_clock::time_point now) const {
    const std::int64_t currentStep =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() / kStepSeconds;

    // Accepting the neighbouring steps keeps the feature usable if the local clock drifts slightly.
    for (std::int64_t offset = -1; offset <= 1; ++offset) {
        if (generateCodeForStep(secret, currentStep + offset) == submittedCode) {
            return true;
        }
    }

    return false;
}

std::string MfaService::generateCurrentCode(
    const std::string& secret,
    std::chrono::system_clock::time_point now) const {
    const std::int64_t currentStep =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() / kStepSeconds;
    return generateCodeForStep(secret, currentStep);
}

std::string MfaService::generateSecret() const {
    return cryptoService_.generateRandomBytes(20);
}

std::string MfaService::generateCodeForStep(const std::string& secret, std::int64_t step) const {
    const std::string digest = cryptoService_.hashSecret(secret, std::to_string(step));
    unsigned int accumulator = 0;
    for (std::size_t index = 0; index < 4 && index < digest.size(); ++index) {
        accumulator = (accumulator << 8) + static_cast<unsigned char>(digest[index]);
    }

    accumulator %= 1000000;

    std::ostringstream stream;
    stream << std::setw(6) << std::setfill('0') << accumulator;
    return stream.str();
}

}  // namespace password_manager_backend
