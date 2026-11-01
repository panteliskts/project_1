#include "core/ivf/ivf.hpp"

#include <algorithm>
#include <cmath>

#include "core/clustering/kmeans.hpp"

void InvertedFile::train(const Dataset& data, int kclusters, uint64_t seed) {
    // Αν δεν δοθεί kclusters, παίρνει default τη ρίζα του n, όπως λέει η εκφώνηση
    if (kclusters <= 0) kclusters = int(std::ceil(std::sqrt(double(data.n))));
    KMeansResult km = kmeans(data, kclusters, seed);
    centroids = std::move(km.centroids);
    assignment = std::move(km.assignment);
    lists.assign(centroids.n, {});
    for (int i = 0; i < data.n; ++i) lists[assignment[i]].push_back(i);
}

// Ταξινομεί τα κέντρα κατά απόσταση από το query και κρατά τα nprobe πλησιέστερα
std::vector<int> InvertedFile::probe(const float* query, int nprobe) const {
    std::vector<Neighbor> byDist(centroids.n);
    for (int c = 0; c < centroids.n; ++c)
        byDist[c] = {c, squaredL2(query, centroids.row(c), centroids.dim)};
    std::vector<Neighbor> nearest = topNeighbors(byDist, nprobe);
    std::vector<int> ids;
    for (const Neighbor& nb : nearest) ids.push_back(nb.id);
    return ids;
}