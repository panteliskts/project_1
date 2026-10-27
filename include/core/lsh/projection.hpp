#pragma once

#include <cstdint>
#include <vector>

#include "core/common/random.hpp"

class ProjectionHashes {
public:
    ProjectionHashes() = default;
    ProjectionHashes(int count, int dim, double w, Random& random);

    int64_t operator()(int j, const float* p) const;

private:
    int dim_ = 0;
    double w_ = 1.0;
    std::vector<float> v_;   // count x dim
    std::vector<double> t_;  // count
};