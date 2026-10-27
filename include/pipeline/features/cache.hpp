#pragma once

#include <string>
#include <vector>

#include "core/common/vec.hpp"

struct ImageSet {
    std::vector<std::string> ids;  // image_id
    Dataset vectors;               // one row per id
    long scanned = 0;              // files examined so far, including skipped
};

bool loadDataset(const std::string& path, Dataset& out);
void saveDataset(const std::string& path, const Dataset& data);
bool loadImageSet(const std::string& path, ImageSet& out);
void saveImageSet(const std::string& path, const ImageSet& set);