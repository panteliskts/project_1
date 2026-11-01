#pragma once

#include <cstdint>
#include <vector>

#include "core/common/vec.hpp"

// Ανεστραμμένο ευρετήριο: k-means σε kclusters κέντρα, κάθε κέντρο κρατά λίστα με τα σημεία του
struct InvertedFile {
    Dataset centroids;
    std::vector<int> assignment;           // centroid of every base vector
    std::vector<std::vector<int>> lists;   // base ids per centroid

    // kclusters
    void train(const Dataset& data, int kclusters, uint64_t seed);

    // The nprobe centroids closest to query
    // Επιστρέφει τα nprobe πλησιέστερα κέντρα στο query (όχι τα ίδια τα σημεία)
    std::vector<int> probe(const float* query, int nprobe) const;
};