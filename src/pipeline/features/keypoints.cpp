#include "pipeline/features/keypoints.hpp"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <stdexcept>

#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>

#include "core/common/parallel.hpp"
#include "pipeline/features/cache.hpp"
#include "pipeline/features/features.hpp"

namespace fs = std::filesystem;

namespace {

const int kMaxKeypoints = 2000;

bool extract(const std::string& path, Dataset& table) {
    cv::Mat image = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (image.empty()) return false;
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat desc;
    cv::SIFT::create(kMaxKeypoints)->detectAndCompute(image, cv::noArray(), keypoints, desc);
    table = Dataset(int(keypoints.size()) + 1, 130);
    table.row(0)[0] = float(image.cols);
    table.row(0)[1] = float(image.rows);
    for (int i = 0; i < desc.rows; ++i) {
        float* r = table.row(i + 1);
        r[0] = keypoints[i].pt.x;
        r[1] = keypoints[i].pt.y;
        std::copy_n(desc.ptr<float>(i), 128, r + 2);
    }
    return true;
}

ImageFeatures unpack(const Dataset& table) {
    ImageFeatures f;
    f.width = int(table.row(0)[0]);
    f.height = int(table.row(0)[1]);
    f.desc = Dataset(table.n - 1, 128);
    for (int i = 0; i < f.desc.n; ++i) {
        const float* r = table.row(i + 1);
        f.points.emplace_back(r[0], r[1]);
        std::copy_n(r + 2, 128, f.desc.row(i));
    }
    return f;
}

} 

std::vector<ImageFeatures> loadOrExtractFeatures(const std::vector<std::string>& paths, const std::string& cacheDir) {
    cv::setNumThreads(0);
    std::string dir = cacheDir.empty() ? "" : cacheDir + "/sift" + std::to_string(kMaxKeypoints);
    if (!dir.empty()) fs::create_directories(dir);
    std::vector<ImageFeatures> features(paths.size());
    std::vector<char> ok(paths.size(), 1);
    parallelFor(0, int(paths.size()), [&](int i) {
        std::string file = dir + "/" + std::to_string(std::hash<std::string>()(fs::weakly_canonical(paths[i]).string()));
        Dataset table;
        if (dir.empty() || !loadDataset(file, table)) {
            ok[i] = extract(paths[i], table);
            if (ok[i] && !dir.empty()) saveDataset(file, table);
        }
        if (ok[i]) features[i] = unpack(table);
    }, kHPatchesSiftThreads);
    for (size_t i = 0; i < paths.size(); ++i)
        if (!ok[i]) throw std::runtime_error("cannot read image " + paths[i]);
    return features;
}