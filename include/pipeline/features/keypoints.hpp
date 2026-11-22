#pragma once

#include <string>
#include <vector>

#include "core/common/vec.hpp"
#include "core/geometry/hmgraph.hpp"

struct ImageFeatures {
    int width = 0;
    int height = 0;
    std::vector<Point2> points;
    Dataset desc;
};

std::vector<ImageFeatures> loadOrExtractFeatures(const std::vector<std::string>& paths, const std::string& cacheDir);