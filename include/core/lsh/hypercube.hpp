#pragma once

#include <cstdint>
#include <unordered_map>

#include "core/lsh/projection.hpp"
#include "core/common/search_index.hpp"

struct HypercubeParams {
    int kproj = 10;   // dimension d' of the hypercube
    double w = 0.05;  // cell length on the projection line
    int M = 500;      // max candidate points checked
    int probes = 10;  // max hypercube vertices 
    uint64_t seed = 1;
};

// Random projection on the hypercube {0,1}^d'
// Προβάλλει κάθε σημείο σε μια κορυφή υπερκύβου, η αναζήτηση ψάχνει κοντινές κορυφές με απόσταση Hamming
class HypercubeIndex : public SearchIndex {
public:
    explicit HypercubeIndex(const HypercubeParams& params);

    std::string name() const override { return "Hypercube"; }
    void build(const Dataset& data) override;
    std::vector<Neighbor> search(const float* query, int topN) const override;

private:
    uint64_t vertex(const float* x) const;

    HypercubeParams p_;
    const Dataset* data_ = nullptr;
    ProjectionHashes h_;
    uint64_t coinSalt_ = 0;  

    std::unordered_map<uint64_t, std::vector<int>> vertices_;
};