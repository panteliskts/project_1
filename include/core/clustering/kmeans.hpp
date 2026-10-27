#pragma once

#include <cstdint>
#include <vector>

#include "core/common/vec.hpp"

struct KMeansResult {
    Dataset centroids;            // dimentions
    std::vector<int> assignment;  // cluster of every input point
};

KMeansResult kmeans(const Dataset& points, int k, uint64_t seed, int maxIters = 30);

// Index closest to x
int nearestCentroid(const Dataset& centroids, const float* x, float* sqDist = nullptr);