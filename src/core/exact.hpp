#pragma once

#include "search_index.hpp"

// Brute forcing
class ExactSearch : public SearchIndex {
public:
    std::string name() const override { return "Exact Search"; }
    void build(const Dataset& data) override { data_ = &data; }
    std::vector<Neighbor> search(const float* query, int topN) const override;

private:
    const Dataset* data_ = nullptr;
};