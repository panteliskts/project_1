#pragma once

#include <cstdint>
#include <unordered_map>

#include "core/lsh/projection.hpp"
#include "core/common/search_index.hpp"

struct LSHParams {
    int k = 4;       // functions h_i combined into every g
    int L = 5;       // hash tables
    double w = 0.05; // cell length on the projection line
    uint64_t seed = 1;
};

// Euclidean LSH with L tables
class LSHIndex : public SearchIndex {
public:
    explicit LSHIndex(const LSHParams& params);

    std::string name() const override { return "LSH"; }
    void build(const Dataset& data) override;
    std::vector<Neighbor> search(const float* query, int topN) const override;

private:
    uint64_t bucketKey(int table, const float* x) const;

    LSHParams p_;
    const Dataset* data_ = nullptr;
    ProjectionHashes h_;         // L*k functions, table l uses [l*k, l*k + k)
    std::vector<uint64_t> r_;    // L*k random multipliers
    std::vector<std::unordered_map<uint64_t, std::vector<int>>> tables_;
};