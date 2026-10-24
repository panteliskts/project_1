#include "cache.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <stdexcept>

namespace {

const uint32_t kDatasetMagic = 0x56454331;  
const uint32_t kImageSetMagic = 0x494d4731;  

template <class T>
void put(std::ofstream& out, const T& v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

template <class T>
bool get(std::ifstream& in, T& v) {
    return bool(in.read(reinterpret_cast<char*>(&v), sizeof(T)));
}

void putDataset(std::ofstream& out, const Dataset& d) {
    put(out, int32_t(d.n));
    put(out, int32_t(d.dim));
    out.write(reinterpret_cast<const char*>(d.data.data()), std::streamsize(d.data.size() * sizeof(float)));
}

bool getDataset(std::ifstream& in, Dataset& d) {
    int32_t n, dim;
    if (!get(in, n) || !get(in, dim) || n < 0 || dim < 0) return false;
    d = Dataset(n, dim);
    return bool(in.read(reinterpret_cast<char*>(d.data.data()), std::streamsize(d.data.size() * sizeof(float))));
}

template <class Fn>
void writeAtomically(const std::string& path, Fn body) {
    std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary);
        if (!out) throw std::runtime_error("cannot write " + tmp);
        body(out);
        if (!out) throw std::runtime_error("error while writing " + tmp);
    }
    if (std::rename(tmp.c_str(), path.c_str()) != 0) throw std::runtime_error("cannot rename " + tmp);
}

}  

bool loadDataset(const std::string& path, Dataset& out) {
    std::ifstream in(path, std::ios::binary);
    uint32_t magic;
    return in && get(in, magic) && magic == kDatasetMagic && getDataset(in, out);
}

void saveDataset(const std::string& path, const Dataset& data) {
    writeAtomically(path, [&](std::ofstream& out) {
        put(out, kDatasetMagic);
        putDataset(out, data);
    });
}

bool loadImageSet(const std::string& path, ImageSet& out) {
    std::ifstream in(path, std::ios::binary);
    uint32_t magic;
    int64_t scanned;
    if (!in || !get(in, magic) || magic != kImageSetMagic || !get(in, scanned)) return false;
    if (!getDataset(in, out.vectors)) return false;
    out.scanned = long(scanned);
    out.ids.assign(out.vectors.n, "");
    for (std::string& id : out.ids) {
        uint32_t len;
        if (!get(in, len) || len > 4096) return false;
        id.resize(len);
        if (!in.read(&id[0], len)) return false;
    }
    return true;
}

void saveImageSet(const std::string& path, const ImageSet& set) {
    writeAtomically(path, [&](std::ofstream& out) {
        put(out, kImageSetMagic);
        put(out, int64_t(set.scanned));
        putDataset(out, set.vectors);
        for (const std::string& id : set.ids) {
            put(out, uint32_t(id.size()));
            out.write(id.data(), std::streamsize(id.size()));
        }
    });
    
}
