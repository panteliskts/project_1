#include "cli.hpp"

#include <stdexcept>

namespace {

int toInt(const std::string& flag, const std::string& value) {
    size_t used = 0;
    int v = 0;
    try {
        v = std::stoi(value, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != value.size()) throw std::runtime_error(flag + " expects an integer, got '" + value + "'");
    return v;
}

double toDouble(const std::string& flag, const std::string& value) {
    size_t used = 0;
    double v = 0.0;
    try {
        v = std::stod(value, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != value.size()) throw std::runtime_error(flag + " expects a number, got '" + value + "'");
    return v;
}

void selectMethod(Options& o, Method m) {
    if (o.method != Method::None && o.method != m) throw std::runtime_error("choose exactly one search method");
    o.method = m;
}

}  

std::string usage() {
    return "usage: ./search -hp <dir> -mir <dir> -split <file> -set <validation|test> -o <file>\n"
           "                [-vocab K] [-S int] [-D int] [-seed int] [-cache <dir>]\n"
           "                -exact\n"
           "              | -lsh       [-k int] [-L int] [-w double]\n"
           "              | -hypercube [-kproj int] [-w double] [-M int] [-probes int]\n"
           "              | -ivfflat   [-kclusters int] [-nprobe int]\n"
           "              | -ivfpq     [-kclusters int] [-nprobe int] [-M int] [-nbits int]\n";
}

Options parseArgs(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        std::string flag = argv[i];

        if (flag == "-exact") { selectMethod(o, Method::Exact); continue; }
        if (flag == "-lsh") { selectMethod(o, Method::LSH); continue; }
        if (flag == "-hypercube") { selectMethod(o, Method::Hypercube); continue; }
        if (flag == "-ivfflat") { selectMethod(o, Method::IVFFlat); continue; }
        if (flag == "-ivfpq") { selectMethod(o, Method::IVFPQ); continue; }

        if (i + 1 >= argc) throw std::runtime_error("missing value after " + flag);
        std::string value = argv[++i];

        if (flag == "-hp") o.hpDir = value;
        else if (flag == "-mir") o.mirDir = value;
        else if (flag == "-split") o.splitFile = value;
        else if (flag == "-set") o.set = value;
        else if (flag == "-o") o.outFile = value;
        else if (flag == "-cache") o.cacheDir = value;
        else if (flag == "-vocab") o.vocab = toInt(flag, value);
        else if (flag == "-S") o.S = toInt(flag, value);
        else if (flag == "-D") o.D = toInt(flag, value);
        else if (flag == "-seed") o.seed = uint64_t(toInt(flag, value));
        else if (flag == "-k") o.lsh.k = toInt(flag, value);
        else if (flag == "-L") o.lsh.L = toInt(flag, value);
        else if (flag == "-w") o.lsh.w = o.cube.w = toDouble(flag, value);
        else if (flag == "-kproj") o.cube.kproj = toInt(flag, value);
        else if (flag == "-probes") o.cube.probes = toInt(flag, value);
        else if (flag == "-M") o.cube.M = o.ivfpq.M = toInt(flag, value);
        else if (flag == "-kclusters") o.ivfflat.kclusters = o.ivfpq.kclusters = toInt(flag, value);
        else if (flag == "-nprobe") o.ivfflat.nprobe = o.ivfpq.nprobe = toInt(flag, value);
        else if (flag == "-nbits") o.ivfpq.nbits = toInt(flag, value);
        else throw std::runtime_error("unknown option " + flag);
    }

    if (o.hpDir.empty() || o.splitFile.empty() || o.outFile.empty())
        throw std::runtime_error("-hp, -split and -o are required");
    if (o.set != "validation" && o.set != "test") throw std::runtime_error("-set must be validation or test");
    if (o.D > 0 && o.mirDir.empty()) throw std::runtime_error("-mir is required when D > 0");
    if (o.vocab < 1 || o.S < 1 || o.D < 0) throw std::runtime_error("-vocab and -S must be positive, -D non-negative");
    if (o.method == Method::None) throw std::runtime_error("choose a method: -exact, -lsh, -hypercube, -ivfflat or -ivfpq");

    o.lsh.seed = o.cube.seed = o.ivfflat.seed = o.ivfpq.seed = o.seed;
    return o;
}