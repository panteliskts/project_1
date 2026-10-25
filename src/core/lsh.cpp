#include "lsh.hpp"

#include <algorithm>
#include <stdexcept>

LSHIndex::LSHIndex(const LSHParams& params) : p_(params) {
    if (p_.k < 1 || p_.L < 1 || p_.w <= 0.0)
        throw std::invalid_argument("LSH: k and L must be positive and w > 0");
}

uint64_t LSHIndex::bucketKey(int table, const float* x) const {
    uint64_t key = 0;
    for (int i = table * p_.k; i < (table + 1) * p_.k; ++i)
        key += r_[i] * uint64_t(h_(i, x));
    return key;
}

void LSHIndex::build(const Dataset& data) {
    data_ = &data;
    Rng rng(p_.seed);
    h_ = ProjectionHashes(p_.L * p_.k, data.dim, p_.w, rng);
    r_.resize(size_t(p_.L) * p_.k);
    for (uint64_t& r : r_) r = rng.next() | 1;  // odd, so never a zero multiplier

    tables_.assign(p_.L, {});
    for (int l = 0; l < p_.L; ++l)
        for (int i = 0; i < data.n; ++i) tables_[l][bucketKey(l, data.row(i))].push_back(i);
}

std::vector<Neighbor> LSHIndex::search(const float* query, int topN) const {
    std::vector<int> ids;
    for (int l = 0; l < p_.L; ++l) {
        auto it = tables_[l].find(bucketKey(l, query));
        if (it != tables_[l].end()) ids.insert(ids.end(), it->second.begin(), it->second.end());
    }
    // A point usually shares the query's bucket in several tables.
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

    std::vector<Neighbor> cands;
    cands.reserve(ids.size());
    for (int id : ids) cands.push_back({id, squaredL2(query, data_->row(id), data_->dim)});
    return topNeighbors(cands, topN);
}