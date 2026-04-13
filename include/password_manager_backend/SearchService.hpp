#pragma once

#include "password_manager_backend/Models.hpp"

#include <vector>

namespace password_manager_backend {

class SearchService {
public:
    std::vector<VaultEntryView> search(const std::vector<VaultEntryView>& entries, const SearchQuery& query) const;
};

}  // namespace password_manager_backend
