#include "pipeline/dataset/database.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "pipeline/features/cache.hpp"
#include "pipeline/features/features.hpp"

namespace fs = std::filesystem;

namespace {

std::string hpImageId(const std::string& seq, int k) { return seq + "/" + std::to_string(k); }

std::string hpImagePath(const Options& o, const std::string& seq, int k) {
    return o.hpDir + "/" + seq + "/" + std::to_string(k) + ".ppm";
}

void appendRow(Dataset& d, const float* row) {
    d.data.insert(d.data.end(), row, row + d.dim);
    ++d.n;
}

std::string hash8(const std::string& s) {
    uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : s) h = (h ^ c) * 1099511628211ULL;
    char hex[17];
    std::snprintf(hex, sizeof hex, "%016llx", (unsigned long long)h);
    return std::string(hex, 8);
}

std::string dirId(const std::string& dir) {
    fs::path p = fs::weakly_canonical(fs::absolute(dir));
    if (!p.has_filename()) p = p.parent_path();
    return p.string();
}


std::string cacheKey(const Options& o) {
    std::ifstream in(o.splitFile, std::ios::binary);
    std::ostringstream content;
    content << in.rdbuf();
    return "K" + std::to_string(o.vocab) + "_S" + std::to_string(o.S) + "_seed" + std::to_string(o.seed) +
           "_split" + hash8(content.str()) + "_hp" + hash8(dirId(o.hpDir));
}


std::vector<std::string> listMirFlickr(const std::string& dir) {
    std::vector<std::pair<long, std::string>> files;
    for (const fs::directory_entry& e : fs::directory_iterator(dir)) {
        std::string name = e.path().filename().string();
        if (name.size() < 7 || name.compare(0, 2, "im") != 0 || name.compare(name.size() - 4, 4, ".jpg") != 0) continue;
        std::string digits = name.substr(2, name.size() - 6);
        if (digits.empty() || !std::all_of(digits.begin(), digits.end(), ::isdigit)) continue;
        files.emplace_back(std::stol(digits), name);
    }
    std::sort(files.begin(), files.end());
    std::vector<std::string> names;
    for (auto& f : files) names.push_back(f.second);
    return names;
}

Dataset loadOrTrainVocabulary(const Options& o, const std::vector<SplitEntry>& split, const std::string& key) {
    std::string path = o.cacheDir + "/vocab_" + key + ".bin";
    Dataset vocab;
    if (!o.cacheDir.empty() && loadDataset(path, vocab) && vocab.n == o.vocab) return vocab;

    std::vector<std::string> train;
    for (const SplitEntry& e : split)
        if (e.part == "train")
            for (int k = 1; k <= 6; ++k) train.push_back(hpImagePath(o, e.sequence, k));
    std::fprintf(stderr, "Training vocabulary on %zu images (S=%d)\n", train.size(), o.S);
    vocab = trainVocabulary(train, o.vocab, o.S, o.seed);
    if (!o.cacheDir.empty()) saveDataset(path, vocab);
    return vocab;
}


ImageSet loadOrComputeHPatches(const Options& o, const std::vector<SplitEntry>& split, const Dataset& vocab,
                               const std::string& key) {
    std::string path = o.cacheDir + "/hpatches_" + key + ".bin";
    ImageSet hp;
    if (!o.cacheDir.empty() && loadImageSet(path, hp)) return hp;

    std::vector<std::string> paths;
    for (const SplitEntry& e : split)
        if (e.part != "train")
            for (int k = 1; k <= 6; ++k) {
                hp.ids.push_back(hpImageId(e.sequence, k));
                paths.push_back(hpImagePath(o, e.sequence, k));
            }
    std::fprintf(stderr, "Computing vectors of %zu HPatches images\n", paths.size());
    std::vector<ImageStatus> status;
    hp.vectors = imagesToVectors(paths, vocab, status, kHPatchesSiftThreads);
    for (size_t i = 0; i < paths.size(); ++i) {
        if (status[i] == ImageStatus::Unreadable) throw std::runtime_error("cannot read image " + paths[i]);
        if (status[i] == ImageStatus::NoDescriptors)
            std::fprintf(stderr, "warning: %s has no SIFT descriptors\n", paths[i].c_str());
    }
    if (!o.cacheDir.empty()) saveImageSet(path, hp);
    return hp;
}


ImageSet loadOrExtendMirFlickr(const Options& o, const Dataset& vocab, const std::string& key) {
    std::string path = o.cacheDir + "/mirflickr_" + key + "_mir" + hash8(dirId(o.mirDir)) + ".bin";
    ImageSet mir;
    if (o.cacheDir.empty() || !loadImageSet(path, mir)) {
        mir = ImageSet();
        mir.vectors = Dataset(0, vocab.n);
    }
    if (int(mir.ids.size()) >= o.D) return mir;

    std::vector<std::string> files = listMirFlickr(o.mirDir);
    bool grew = false;
    while (int(mir.ids.size()) < o.D && mir.scanned < long(files.size())) {
        long need = o.D - long(mir.ids.size());
        long batch = std::min(long(files.size()) - mir.scanned, need + need / 50 + 16);
        std::vector<std::string> paths;
        for (long i = mir.scanned; i < mir.scanned + batch; ++i) paths.push_back(o.mirDir + "/" + files[i]);
        std::fprintf(stderr, "Computing vectors of MIRFlickr files %ld..%ld\n", mir.scanned + 1, mir.scanned + batch);

        std::vector<ImageStatus> status;
        Dataset vecs = imagesToVectors(paths, vocab, status);
        for (long i = 0; i < batch; ++i) {
            if (status[i] != ImageStatus::Ok) {
                std::fprintf(stderr, "  skipping %s (%s)\n", files[mir.scanned + i].c_str(),
                             status[i] == ImageStatus::Unreadable ? "unreadable" : "no SIFT descriptors");
                continue;
            }
            mir.ids.push_back(files[mir.scanned + i]);
            appendRow(mir.vectors, vecs.row(int(i)));
        }
        mir.scanned += batch;
        grew = true;
    }
    if (grew && !o.cacheDir.empty()) saveImageSet(path, mir);
    return mir;
}

}

