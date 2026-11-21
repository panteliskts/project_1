#include "core/geometry/hmgraph.hpp"

#include <limits>

namespace {


bool normalization(const std::vector<Point2>& pts, Eigen::Matrix3d& T) {
    Point2 c = Point2::Zero();
    for (const Point2& p : pts) c += p;
    c /= double(pts.size());
    double meanDist = 0.0;
    for (const Point2& p : pts) meanDist += dist(p, c);
    meanDist /= double(pts.size());
    if (!(meanDist > 1e-12)) return false;
    double s = std::sqrt(2.0) / meanDist;
    T << s, 0.0, -s * c.x(),
         0.0, s, -s * c.y(),
         0.0, 0.0, 1.0;
    return true;
}

Point2 apply(const Eigen::Matrix3d& T, const Point2& p) {
    return {T(0, 0) * p.x() + T(0, 1) * p.y() + T(0, 2), T(1, 0) * p.x() + T(1, 1) * p.y() + T(1, 2)};
}

} 

bool fitHomography(const std::vector<Point2>& src, const std::vector<Point2>& dst, Eigen::Matrix3d& H) {
    const int n = int(src.size());
    Eigen::Matrix3d T, Tp;
    if (n < 4 || int(dst.size()) != n || !normalization(src, T) || !normalization(dst, Tp)) return false;

    Eigen::MatrixXd A(2 * n, 9);
    for (int i = 0; i < n; ++i) {
        Point2 p = apply(T, src[i]), q = apply(Tp, dst[i]);
        double x = p.x(), y = p.y(), u = q.x(), v = q.y();
        A.row(2 * i) << -x, -y, -1.0, 0.0, 0.0, 0.0, u * x, u * y, u;
        A.row(2 * i + 1) << 0.0, 0.0, 0.0, -x, -y, -1.0, v * x, v * y, v;
    }

#if EIGEN_VERSION_AT_LEAST(3, 4, 90)
    Eigen::JacobiSVD<Eigen::MatrixXd, Eigen::ComputeFullV> svd(A);
#else
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeFullV);
#endif
    Eigen::VectorXd h = svd.matrixV().col(8);
    Eigen::Matrix3d Hn;
    Hn << h(0), h(1), h(2), h(3), h(4), h(5), h(6), h(7), h(8);

  
    H = Tp.inverse() * Hn * T;
    H /= H.norm();
    return isValidHomography(H);
}

bool project(const Eigen::Matrix3d& H, const Point2& p, Point2& out) {
    double a = H(0, 0) * p.x() + H(0, 1) * p.y() + H(0, 2);
    double b = H(1, 0) * p.x() + H(1, 1) * p.y() + H(1, 2);
    double c = H(2, 0) * p.x() + H(2, 1) * p.y() + H(2, 2);
    double scale = std::abs(H(2, 0) * p.x()) + std::abs(H(2, 1) * p.y()) + std::abs(H(2, 2));
    if (!(std::abs(c) > 1e-10 * scale)) return false;
    out = Point2(a / c, b / c);
    return out.allFinite();
}

bool isValidHomography(const Eigen::Matrix3d& H) {
    if (!H.allFinite()) return false;
    Eigen::Vector3d s = Eigen::JacobiSVD<Eigen::Matrix3d>(H).singularValues();
    return s(0) > 0.0 && s(2) > 1e-10 * s(0);
}

double cornerError(const Eigen::Matrix3d& H, const Eigen::Matrix3d& Hgt, int width, int height) {
    double w = width - 1.0, h = height - 1.0, sum = 0.0;
    for (Point2 a : {Point2(0, 0), Point2(w, 0), Point2(w, h), Point2(0, h)}) {
        Point2 p, q;
        if (!project(H, a, p) || !project(Hgt, a, q)) return std::numeric_limits<double>::infinity();
        sum += dist(p, q);
    }
    return sum / 4.0;
}
