#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

#include "core/common/vec.hpp"

// Τι έγινε με μια εικόνα: διαβάστηκε κανονικά, δεν διαβάστηκε, ή δεν είχε SIFT descriptors
enum class ImageStatus : char { Ok, Unreadable, NoDescriptors };

constexpr int kHPatchesSiftThreads = 2;

// Εξάγει τους SIFT descriptors μιας εικόνας
bool extractSift(const std::string& path, cv::Mat& descriptors);

// Εκπαιδεύει το λεξιλόγιο των K κέντρων από τις εικόνες training
Dataset trainVocabulary(const std::vector<std::string>& paths, int K, int S, uint64_t seed);

// Φτιάχνει το ιστόγραμμα οπτικών λέξεων μιας εικόνας από τους descriptors της
void bovwVector(const cv::Mat& descriptors, const Dataset& vocab, float* out);

// Τρέχει το bovwVector σε πολλές εικόνες παράλληλα
Dataset imagesToVectors(const std::vector<std::string>& paths, const Dataset& vocab,
                        std::vector<ImageStatus>& status, int maxThreads = 0);