#include "core/lsh/projection.hpp"

#include <cmath>

ProjectionHashes::ProjectionHashes(int count, int dim, double w, Random& random)
    : dim_(dim), w_(w), v_(size_t(count) * dim), t_(count) {
    for (float& x : v_) x = float(random.normal());
    for (double& t : t_) t = random.uniform() * w;
}

int64_t ProjectionHashes::operator()(int j, const float* p) const {
    const float* v = &v_[size_t(j) * dim_];
    double dot = 0.0;
    for (int d = 0; d < dim_; ++d) dot += double(v[d]) * p[d];
    return int64_t(std::floor((dot + t_[j]) / w_));
}