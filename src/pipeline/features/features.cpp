#include "pipeline/features/features.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <numeric>
#include <stdexcept>

#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>

#include "core/clustering/kmeans.hpp"
#include "core/common/parallel.hpp"
#include "core/common/random.hpp"

namespace {

void disableOpenCvThreads() { cv::setNumThreads(0); }

void progress(const char* what, int done, int total) {
    if (done % 500 == 0 || done == total) std::fprintf(stderr, "\r  %s: %d/%d", what, done, total);
    if (done == total) std::fprintf(stderr, "\n");
}

}  // namespace

bool extractSift(const std::string& path, cv::Mat& descriptors) {
    cv::Mat image = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (image.empty()) return false;
    std::vector<cv::KeyPoint> keypoints;
    cv::SIFT::create()->detectAndCompute(image, cv::noArray(), keypoints, descriptors);
    return true;
}

Dataset trainVocabulary(const std::vector<std::string>& paths, int K, int S, uint64_t seed) {
    disableOpenCvThreads();
    const int n = int(paths.size());
    std::vector<cv::Mat> samples(n);
    std::vector<char> readable(n, 0);
    std::atomic<int> done(0);

    parallelFor(0, n, [&](int i) {
        cv::Mat desc;
        readable[i] = extractSift(paths[i], desc);
        std::vector<int> order(desc.rows);
        std::iota(order.begin(), order.end(), 0);
        int take = std::min(S, desc.rows);
        Random random(mix64(seed) ^ mix64(uint64_t(i) + 1));
        for (int j = 0; j < take; ++j) std::swap(order[j], order[j + int(random.below(desc.rows - j))]);
        samples[i].create(take, 128, CV_32F);
        for (int j = 0; j < take; ++j) desc.row(order[j]).copyTo(samples[i].row(j));
        progress("SIFT on training images", ++done, n);
    }, kHPatchesSiftThreads);
    for (int i = 0; i < n; ++i)
        if (!readable[i]) throw std::runtime_error("cannot read image " + paths[i]);

    int total = 0;
    for (const cv::Mat& s : samples) total += s.rows;
    Dataset points(total, 128);
    int row = 0;
    for (const cv::Mat& s : samples)
        for (int r = 0; r < s.rows; ++r, ++row) std::copy(s.ptr<float>(r), s.ptr<float>(r) + 128, points.row(row));

    std::fprintf(stderr, "  k-means on %d descriptors, K=%d\n", total, K);
    return kmeans(points, K, seed).centroids;
}

void bovwVector(const cv::Mat& descriptors, const Dataset& vocab, float* out) {
    std::fill(out, out + vocab.n, 0.0f);
    if (descriptors.rows == 0) return;
    CV_Assert(descriptors.type() == CV_32F && descriptors.cols == vocab.dim);
    for (int r = 0; r < descriptors.rows; ++r) out[nearestCentroid(vocab, descriptors.ptr<float>(r))] += 1.0f;
    for (int j = 0; j < vocab.n; ++j) out[j] /= float(descriptors.rows);
}

Dataset imagesToVectors(const std::vector<std::string>& paths, const Dataset& vocab,
                        std::vector<ImageStatus>& status, int maxThreads) {
    disableOpenCvThreads();
    const int n = int(paths.size());
    Dataset vectors(n, vocab.n);
    status.assign(n, ImageStatus::Ok);
    std::atomic<int> done(0);

    parallelFor(0, n, [&](int i) {
        cv::Mat desc;
        if (!extractSift(paths[i], desc)) status[i] = ImageStatus::Unreadable;
        else if (desc.rows == 0) status[i] = ImageStatus::NoDescriptors;
        bovwVector(desc, vocab, vectors.row(i));
        progress("SIFT + BoVW", ++done, n);
    }, maxThreads);
    return vectors;
}