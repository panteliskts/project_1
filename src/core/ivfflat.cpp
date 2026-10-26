#include "ivfflat.hpp"

#include <stdexcept>

IVFFlatIndex::IVFFlatIndex(const IVFFlatParams& params) : p_(params) {
    if (p_.nprobe < 1) throw std::invalid_argument("IVFFlat: nprobe must be positive");
}

void IVFFlatIndex::build(const Dataset& data) {
    data_ = &data;
    ivf_.train(data, p_.kclusters, p_.seed);
}

std::vector<Neighbor> IVFFlatIndex::search(const float* query, int topN) const {
    std::vector<Neighbor> cands;
    for (int c : ivf_.probe(query, p_.nprobe))
        for (int id : ivf_.lists[c])
            cands.push_back({id, squaredL2(query, data_->row(id), data_->dim)});
    return topNeighbors(cands, topN);
}