#include "core/clustering/kmeans.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

#include "core/common/parallel.hpp"
#include "core/common/random.hpp"

int nearestCentroid(const Dataset& centroids, const float* x, float* sqDist) {
    int best = 0;
    float bestDist = squaredL2(x, centroids.row(0), centroids.dim);
    for (int c = 1; c < centroids.n; ++c) {
        float d = squaredL2(x, centroids.row(c), centroids.dim);
        if (d < bestDist) {
            bestDist = d;
            best = c;
        }
    }
    if (sqDist) *sqDist = bestDist;
    return best;
}

namespace {

void copyRow(const Dataset& from, int i, Dataset& to, int j) {
    std::copy(from.row(i), from.row(i) + from.dim, to.row(j));
}


template <class Fn>
void forEachPoint(int n, size_t work, Fn fn) {
    if (work < (size_t(1) << 18)) {
        for (int i = 0; i < n; ++i) fn(i);
    } else {
        parallelFor(0, n, fn);
    }
}


// k-means++: όσο πιο μακριά είναι ένα σημείο από τα κέντρα που έχουν ήδη επιλεγεί,
// τόσο πιο πιθανό είναι να επιλεγεί ως το επόμενο κέντρο
Dataset seedCentroids(const Dataset& points, int k, Random& Random) {
    Dataset centroids(k, points.dim);
    std::vector<double> minDist(points.n, std::numeric_limits<double>::max());
    copyRow(points, int(Random.below(points.n)), centroids, 0);

    for (int c = 1; c < k; ++c) {
        const float* last = centroids.row(c - 1);
        forEachPoint(points.n, size_t(points.n) * points.dim, [&](int i) {
            double d = squaredL2(points.row(i), last, points.dim);
            if (d < minDist[i]) minDist[i] = d;
        });
        double total = 0.0;
        for (double d : minDist) total += d;

        int pick = int(Random.below(points.n)); 
        if (total > 0.0) {
            double target = Random.uniform() * total;
            double acc = 0.0;
            pick = points.n - 1;
            for (int i = 0; i < points.n; ++i) {
                acc += minDist[i];
                if (acc > target) {
                    pick = i;
                    break;
                }
            }
        }
        copyRow(points, pick, centroids, c);
    }
    return centroids;
}

} 

KMeansResult kmeans(const Dataset& points, int k, uint64_t seed, int maxIters) {
    if (points.n == 0) throw std::invalid_argument("kmeans: empty input");
    k = std::max(1, std::min(k, points.n));
    const int dim = points.dim;

    Random Random(seed);
    KMeansResult res;
    res.centroids = seedCentroids(points, k, Random);
    res.assignment.assign(points.n, -1);

    std::vector<int> newAssign(points.n);
    std::vector<float> dist(points.n);
    for (int iter = 0;; ++iter) {
        forEachPoint(points.n, size_t(points.n) * k * dim, [&](int i) {
            newAssign[i] = nearestCentroid(res.centroids, points.row(i), &dist[i]);
        });
        int changed = 0;
        for (int i = 0; i < points.n; ++i) changed += newAssign[i] != res.assignment[i];
        res.assignment.swap(newAssign);
        // Σταματά όταν αλλάζουν πολύ λίγες αναθέσεις ή τελειώσουν οι επαναλήψεις
        if (changed <= points.n / 1000 || iter == maxIters) break;

        std::vector<double> sums(size_t(k) * dim, 0.0);
        std::vector<int> counts(k, 0);
        for (int i = 0; i < points.n; ++i) {
            int c = res.assignment[i];
            ++counts[c];
            const float* p = points.row(i);
            double* s = &sums[size_t(c) * dim];
            for (int d = 0; d < dim; ++d) s[d] += p[d];
        }
        for (int c = 0; c < k; ++c) {
            float* centre = res.centroids.row(c);
            if (counts[c] > 0) {
                const double* s = &sums[size_t(c) * dim];
                for (int d = 0; d < dim; ++d) centre[d] = float(s[d] / counts[c]);
            } else {
                // Αν ένα cluster αδειάσει, παίρνει το πιο απομακρυσμένο σημείο ως νέο κέντρο
                int far = int(std::max_element(dist.begin(), dist.end()) - dist.begin());
                copyRow(points, far, res.centroids, c);
                dist[far] = 0.0f;
            }
        }
    }
    return res;
}