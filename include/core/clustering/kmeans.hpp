#pragma once

#include <cstdint>
#include <vector>

#include "core/common/vec.hpp"

struct KMeansResult {
    Dataset centroids;            // dimentions
    std::vector<int> assignment;  // cluster of every input point
};

// Αρχικοποίηση k-means++ και επαναλήψεις Lloyd, επιστρέφει τα κέντρα και σε ποιο κέντρο πάει κάθε σημείο
KMeansResult kmeans(const Dataset& points, int k, uint64_t seed, int maxIters = 30);


// Βρίσκει το κοντινότερο κέντρο στο x, ελέγχοντας όλα τα κέντρα ένα προς ένα
int nearestCentroid(const Dataset& centroids, const float* x, float* sqDist = nullptr);