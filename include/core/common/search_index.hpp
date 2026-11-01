#pragma once

#include <string>
#include <vector>

#include "core/common/vec.hpp"

// Κοινή διεπαφή για όλες τις μεθόδους αναζήτησης: Exact, LSH, Hypercube, IVFFlat, IVFPQ
class SearchIndex {
public:
    virtual ~SearchIndex() = default;

    virtual std::string name() const = 0;

    virtual void build(const Dataset& data) = 0;

    virtual std::vector<Neighbor> search(const float* query, int topN) const = 0;
};
