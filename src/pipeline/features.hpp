#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

#include "../core/vec.hpp"

enum class ImageStatus : char { Ok, Unreadable, NoDescriptors };

constexpr int kHPatchesSiftThreads = 2;

bool extractSift(const std::string& path, cv::Mat& descriptors);


Dataset trainVocabulary(const std::vector<std::string>& paths, int K, int S, uint64_t seed);


void bovwVector(const cv::Mat& descriptors, const Dataset& vocab, float* out);


Dataset imagesToVectors(const std::vector<std::string>& paths, const Dataset& vocab,
                        std::vector<ImageStatus>& status, int maxThreads = 0);