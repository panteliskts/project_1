#include "core/ivf/ivfpq.hpp"

#include <stdexcept>
#include <string>

#include "core/clustering/kmeans.hpp"

IVFPQIndex::IVFPQIndex(const IVFPQParams& params) : p_(params) {
    if (p_.nprobe < 1 || p_.M < 1) throw std::invalid_argument("IVFPQ: nprobe and M must be positive");
    if (p_.nbits < 1 || p_.nbits > 16) throw std::invalid_argument("IVFPQ: nbits must be in [1, 16]");
}

void IVFPQIndex::build(const Dataset& data) {
    if (data.dim % p_.M != 0)
        throw std::invalid_argument("IVFPQ: K=" + std::to_string(data.dim) +
                                    " is not divisible by M=" + std::to_string(p_.M));
    dim_ = data.dim;
    subDim_ = dim_ / p_.M;
    ivf_.train(data, p_.kclusters, p_.seed);

    codebooks_.assign(p_.M, Dataset());
    codes_.assign(size_t(data.n) * p_.M, 0);
    for (int m = 0; m < p_.M; ++m) {
        // Sub-vector m of every residual.
        Dataset sub(data.n, subDim_);
        for (int i = 0; i < data.n; ++i) {
            const float* x = data.row(i) + m * subDim_;
            const float* c = ivf_.centroids.row(ivf_.assignment[i]) + m * subDim_;
            float* out = sub.row(i);
            for (int d = 0; d < subDim_; ++d) out[d] = x[d] - c[d];
        }
        KMeansResult km = kmeans(sub, 1 << p_.nbits, p_.seed + 1 + m);
        for (int i = 0; i < data.n; ++i) codes_[size_t(i) * p_.M + m] = uint16_t(km.assignment[i]);
        codebooks_[m] = std::move(km.centroids);
    }
    ksub_ = codebooks_[0].n; 
}

std::vector<Neighbor> IVFPQIndex::search(const float* query, int topN) const {
    std::vector<Neighbor> cands;
    std::vector<float> residual(dim_);
    std::vector<float> table(size_t(p_.M) * ksub_);

    for (int c : ivf_.probe(query, p_.nprobe)) {
        const float* centre = ivf_.centroids.row(c);
        for (int d = 0; d < dim_; ++d) residual[d] = query[d] - centre[d];
        for (int m = 0; m < p_.M; ++m)
            for (int j = 0; j < ksub_; ++j)
                table[size_t(m) * ksub_ + j] =
                    squaredL2(residual.data() + m * subDim_, codebooks_[m].row(j), subDim_);

        for (int id : ivf_.lists[c]) {
            const uint16_t* code = &codes_[size_t(id) * p_.M];
            float dist = 0.0f;
            for (int m = 0; m < p_.M; ++m) dist += table[size_t(m) * ksub_ + code[m]];
            cands.push_back({id, dist});
        }
    }
    return topNeighbors(cands, topN);
}