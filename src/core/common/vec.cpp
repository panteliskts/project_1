#include "core/common/vec.hpp"

#include <algorithm>
#include <cmath>

float squaredL2(const float* a, const float* b, int dim) {
    float s0 = 0.0f, s1 = 0.0f, s2 = 0.0f, s3 = 0.0f;
    int i = 0;
    for (; i + 4 <= dim; i += 4) {
        float d0 = a[i] - b[i];
        float d1 = a[i + 1] - b[i + 1];
        float d2 = a[i + 2] - b[i + 2];
        float d3 = a[i + 3] - b[i + 3];
        s0 += d0 * d0;
        s1 += d1 * d1;
        s2 += d2 * d2;
        s3 += d3 * d3;
    }
    for (; i < dim; ++i) {
        float d = a[i] - b[i];
        s0 += d * d;
    }
    return (s0 + s1) + (s2 + s3);
}

std::vector<Neighbor> topNeighbors(std::vector<Neighbor>& cands, int topN) {
    auto closer = [](const Neighbor& x, const Neighbor& y) {
        return x.dist < y.dist || (x.dist == y.dist && x.id < y.id);
    };
    size_t keep = std::min(cands.size(), size_t(std::max(topN, 0)));
    std::partial_sort(cands.begin(), cands.begin() + keep, cands.end(), closer);
    std::vector<Neighbor> out(cands.begin(), cands.begin() + keep);
    for (Neighbor& nb : out) nb.dist = std::sqrt(nb.dist);
    return out;
}
