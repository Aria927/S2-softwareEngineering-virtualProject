#include "password_manager_backend/SearchService.hpp"

#include <algorithm>
#include <cctype>

namespace password_manager_backend {
namespace {

// Search is case-insensitive so GUI callers do not need to pre-normalise text.
std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool matches(const VaultEntryView& entry, const std::string& query) {
    // Search covers the main descriptive fields plus tags to keep the feature useful without adding query complexity.
    const std::string haystack =
        toLower(entry.title + " " + entry.site + " " + entry.username + " " + entry.notes);

    if (haystack.find(query) != std::string::npos) {
        return true;
    }

    return std::any_of(entry.tags.begin(), entry.tags.end(), [&](const std::string& tag) {
        return toLower(tag).find(query) != std::string::npos;
    });
}

}  // namespace

std::vector<VaultEntryView> SearchService::search(
    const std::vector<VaultEntryView>& entries,
    const SearchQuery& query) const {
    if (query.text.empty()) {
        // An empty search behaves like "show all" because that is the most practical default for the GUI.
        return entries;
    }

    const std::string loweredQuery = toLower(query.text);
    std::vector<VaultEntryView> results;

    for (const auto& entry : entries) {
        if (matches(entry, loweredQuery)) {
            results.push_back(entry);
        }
    }

    return results;
}

}  // namespace password_manager_backend
