#pragma once

#include <string>
#include <vector>

#include "core/common/vec.hpp"

// Τα διανύσματα ενός συνόλου εικόνων, με το id της κάθε εικόνας
struct ImageSet {
    std::vector<std::string> ids;  // image_id
    Dataset vectors;               // one row per id
    long scanned = 0;              // files examined so far, including skipped
};

// Αποθήκευση και φόρτωση από δυαδικό αρχείο, ώστε να μην ξαναϋπολογίζονται τα διανύσματα σε κάθε εκτέλεση
bool loadDataset(const std::string& path, Dataset& out);
void saveDataset(const std::string& path, const Dataset& data);
bool loadImageSet(const std::string& path, ImageSet& out);
void saveImageSet(const std::string& path, const ImageSet& set);