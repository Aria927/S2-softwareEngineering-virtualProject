#pragma once

#include "password_manager_backend/Models.hpp"

#include <vector>

namespace password_manager_backend {

// Keeps search behaviour isolated so filtering logic does not clutter the vault service.
class SearchService {
public:
    std::vector<VaultEntryView> search(const std::vector<VaultEntryView>& entries, const SearchQuery& query) const;
};

}  // namespace password_manager_backend
