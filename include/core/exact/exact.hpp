#pragma once

#include "core/common/search_index.hpp"

// Brute forcing
// Ελέγχει όλα τα σημεία ένα προς ένα, χωρίς καμία προσέγγιση
class ExactSearch : public SearchIndex {
public:
    std::string name() const override { return "Exact Search"; }
    void build(const Dataset& data) override { data_ = &data; }
    std::vector<Neighbor> search(const float* query, int topN) const override;

private:
    const Dataset* data_ = nullptr;
};