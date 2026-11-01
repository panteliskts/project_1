#pragma once

#include <cstddef>
#include <vector>

// Πίνακας δεδομένων με n γραμμές και dim στήλες, αποθηκευμένος σε μία σειρά (row-major)
struct Dataset {
    int n = 0;
    int dim = 0;
    std::vector<float> data;

    Dataset() = default;
    Dataset(int n, int dim) : n(n), dim(dim), data(size_t(n) * dim, 0.0f) {}

    const float* row(int i) const { return data.data() + size_t(i) * dim; }
    float* row(int i) { return data.data() + size_t(i) * dim; }
};

struct Neighbor {
    int id;
    float dist;
};

// Το τετράγωνο της ευκλείδειας απόστασης, χωρίς ρίζα, αρκεί για να συγκρίνουμε
float squaredL2(const float* a, const float* b, int dim);

// Οι topN πλησιέστεροι γείτονες, ταξινομημένοι από τον πιο κοντινό
std::vector<Neighbor> topNeighbors(std::vector<Neighbor>& cands, int topN);
