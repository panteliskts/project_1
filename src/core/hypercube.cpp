#include "hypercube.hpp"

#include <stdexcept>

HypercubeIndex::HypercubeIndex(const HypercubeParams& params) : p_(params) {
    if (p_.kproj < 1 || p_.kproj > 30)
        throw std::invalid_argument("Hypercube: kproj must be in [1, 30]");
    if (p_.w <= 0.0 || p_.M < 1 || p_.probes < 1)
        throw std::invalid_argument("Hypercube: w, M and probes must be positive");
}

uint64_t HypercubeIndex::vertex(const float* x) const {
    uint64_t v = 0;
    for (int j = 0; j < p_.kproj; ++j) {
        uint64_t coin = mix64(mix64(coinSalt_ + uint64_t(j)) ^ uint64_t(h_(j, x)));
        v |= (coin & 1) << j;
    }
    return v;
}

void HypercubeIndex::build(const Dataset& data) {
    data_ = &data;
    Random Random(p_.seed);
    h_ = ProjectionHashes(p_.kproj, data.dim, p_.w, Random);
    coinSalt_ = Random.next();
    vertices_.clear();
    for (int i = 0; i < data.n; ++i) vertices_[vertex(data.row(i))].push_back(i);
}

std::vector<Neighbor> HypercubeIndex::search(const float* query, int topN) const {
    const uint64_t home = vertex(query);
    const uint64_t numVertices = uint64_t(1) << p_.kproj;
    std::vector<Neighbor> cands;
    int probed = 0;

    auto visit = [&](uint64_t v) {
        auto it = vertices_.find(v);
        if (it != vertices_.end())
            for (int id : it->second) {
                if (int(cands.size()) == p_.M) break;
                cands.push_back({id, squaredL2(query, data_->row(id), data_->dim)});
            }
        ++probed;
        return probed < p_.probes && int(cands.size()) < p_.M;
    };

    // Hamming radius r
    bool more = visit(home);
    for (int r = 1; r <= p_.kproj && more; ++r) {
        for (uint64_t mask = (uint64_t(1) << r) - 1; mask < numVertices && more;) {
            more = visit(home ^ mask);
            uint64_t low = mask & (~mask + 1);
            uint64_t ripple = mask + low;
            mask = (((ripple ^ mask) >> 2) / low) | ripple;
        }
    }
    return topNeighbors(cands, topN);
}