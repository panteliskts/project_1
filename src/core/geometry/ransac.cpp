#include "core/geometry/ransac.hpp"

#include <algorithm>
#include <cmath>

#include "core/common/random.hpp"

namespace {

bool degenerate(const Point2 (&s)[4]) {
    for (int i = 0; i < 4; ++i)
        for (int j = i + 1; j < 4; ++j) {
            if (dist(s[i], s[j]) < 1e-6) return true;
            for (int k = j + 1; k < 4; ++k) {
                Point2 u = s[j] - s[i], v = s[k] - s[i];
                if (std::abs(u.x() * v.y() - u.y() * v.x()) <= 0.01 * dist(s[j], s[i]) * dist(s[k], s[i])) return true;
            }
        }
    return false;
}

std::vector<int> inliersOf(const Eigen::Matrix3d& H, const std::vector<Point2>& src, const std::vector<Point2>& dst,
                           double tau, double& median) {
    std::vector<int> in;
    std::vector<double> e;
    for (size_t i = 0; i < src.size(); ++i) {
        Point2 q;
        double ei = project(H, src[i], q) ? dist(dst[i], q) : tau;  
        if (!(ei < tau)) continue;
        in.push_back(int(i));
        e.push_back(ei);
    }
    std::sort(e.begin(), e.end());
    size_t m = e.size();
    median = m == 0 ? std::numeric_limits<double>::infinity() : m % 2 ? e[m / 2] : (e[m / 2 - 1] + e[m / 2]) / 2.0;
    return in;
}

} 

int adaptiveIterations(double w, double p, int maxIters) {
    if (w >= 1.0) return 1;  
    double t = std::ceil(std::log(1.0 - p) / std::log(1.0 - std::pow(w, 4)));
    return t > 0.0 && t < maxIters ? int(t) : maxIters;
}

RansacResult ransacHomography(const std::vector<Point2>& src, const std::vector<Point2>& dst,
                              const RansacParams& params) {
    RansacResult res;
    const int M = int(src.size());
    if (M < 4) return res;

    Random rng(params.seed);
    std::vector<int> best;
    double bestMedian = 0.0;
    Eigen::Matrix3d bestH;
    for (int it = 0, limit = params.maxIters; it < limit; ++it) {
        int idx[4];
        for (int j = 0; j < 4; ++j)
            do idx[j] = int(rng.below(uint64_t(M)));
            while (std::find(idx, idx + j, idx[j]) != idx + j);
        Point2 s[4], d[4];
        for (int j = 0; j < 4; ++j) {
            s[j] = src[idx[j]];
            d[j] = dst[idx[j]];
        }
        Eigen::Matrix3d H;
        if (degenerate(s) || degenerate(d) || !fitHomography({s, s + 4}, {d, d + 4}, H)) continue;

        double median;
        std::vector<int> in = inliersOf(H, src, dst, params.tau, median);
        if (in.size() < best.size() || (in.size() == best.size() && !(median < bestMedian))) continue;
        best = in;
        bestMedian = median;
        bestH = H;
        if (params.adaptive) limit = adaptiveIterations(double(best.size()) / M, params.p, params.maxIters);
    }
    if (best.size() < 4) return res;


    std::vector<Point2> inSrc, inDst;
    for (int i : best) {
        inSrc.push_back(src[i]);
        inDst.push_back(dst[i]);
    }
    if (!fitHomography(inSrc, inDst, res.H)) res.H = bestH;  

    double median;
    int n = int(inliersOf(res.H, src, dst, params.tau, median).size());
    if (n < 4) return RansacResult();
    res.valid = true;
    res.inliers = n;
    res.medianError = median;
    return res;
}
