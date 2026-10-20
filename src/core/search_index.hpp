#pragma once

#include <string>
#include <vector>

#include "vec.hpp"

class SearchIndex {
public:
    virtual ~SearchIndex() = default;

    virtual std::string name() const = 0;

    virtual void build(const Dataset& data) = 0;

    virtual std::vector<Neighbor> search(const float* query, int topN) const = 0;
};
