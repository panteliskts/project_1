#pragma once

#include <cstddef>
#include <vector>

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

float squaredL2(const float* a, const float* b, int dim);

std::vector<Neighbor> topNeighbors(std::vector<Neighbor>& cands, int topN);
