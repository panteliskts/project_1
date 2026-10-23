#pragma once

#include <cstdint>
#include <unordered_map>

#include "projection.hpp"
#include "search_index.hpp"

struct HypercubeParams {
    int kproj = 10;   // dimension 
    double w = 0.05;  // cell length
    int M = 500;      // max candidate points checked
    int probes = 10;  // max vertices of hypercube checked
    uint64_t seed = 1;
};