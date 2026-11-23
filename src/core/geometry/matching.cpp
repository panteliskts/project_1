#include "core/geometry/matching.hpp"

#include <algorithm>
#include <limits>

#include <Eigen/Dense>

using RowMatrix = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

std::vector<Match> ratioMatches(const Dataset& descQ, const Dataset& descI, double rho) {
    std::vector<Match> matches;
    if (descQ.n == 0 || descI.n < 2) return matches;
    
    Eigen::Map<const RowMatrix> Q(descQ.data.data(), descQ.n, descQ.dim);
    Eigen::Map<const RowMatrix> I(descI.data.data(), descI.n, descI.dim);
    Eigen::VectorXf qNorm = Q.rowwise().squaredNorm(), iNorm = I.rowwise().squaredNorm();
    RowMatrix dots = Q * I.transpose();

    const float rho2 = float(rho * rho);
    for (int q = 0; q < descQ.n; ++q) {
        float d1 = std::numeric_limits<float>::infinity(), d2 = d1;
        int nearest = -1;
        for (int i = 0; i < descI.n; ++i) {
            float d = std::max(0.0f, qNorm[q] + iNorm[i] - 2.0f * dots(q, i));
            if (d < d1) {
                d2 = d1;
                d1 = d;
                nearest = i;
            } else if (d < d2) {
                d2 = d;
            }
        }
        if (d1 < rho2 * d2) matches.push_back({q, nearest});
    }
    return matches;
}
