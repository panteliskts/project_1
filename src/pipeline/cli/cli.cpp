#include "pipeline/cli/cli.hpp"

#include <stdexcept>

#include "core/exact/exact.hpp"

namespace {

// Μετατρέπει το string σε ακέραιο, με μήνυμα λάθους αν δεν είναι έγκυρος αριθμός
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

// Το ίδιο για δεκαδικό αριθμό
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

// Επιτρέπει μόνο μία μέθοδο αναζήτησης ανά εκτέλεση
void selectMethod(Options& o, Method m) {
    if (o.method != Method::None && o.method != m) throw std::runtime_error("choose exactly one search method");
    o.method = m;
}

}  

std::string usage(bool verify) {
    std::string text = std::string("usage: ") + (verify ? "./verify" : "./search") +
           " -hp <dir> -mir <dir> -split <file> -set <validation|test> -o <file>\n"
           "                [-vocab K] [-S int] [-D int] [-seed int] [-cache <dir>]\n"
           "                -exact\n"
           "              | -lsh       [-k int] [-L int] [-w double]\n"
           "              | -hypercube [-kproj int] [-w double] [-M int] [-probes int]\n"
           "              | -ivfflat   [-kclusters int] [-nprobe int]\n"
           "              | -ivfpq     [-kclusters int] [-nprobe int] [-M int] [-nbits int]\n";
    if (verify)
        text += "                [-C int] [-rho double] [-tau double] [-T int] [-p double] [-adaptive 0|1]\n"
                "                [-rerank inliers|matches]\n"
                "                (-kclusters is required for -ivfflat and -ivfpq)\n";
    return text;
}

// Διαβάζει όλα τα flags, γεμίζει το Options και ελέγχει ότι δόθηκαν τα απαραίτητα
Options parseArgs(int argc, char** argv, bool verify) {
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
        else if (verify && flag == "-C") o.C = toInt(flag, value);
        else if (verify && flag == "-rho") o.rho = toDouble(flag, value);
        else if (verify && flag == "-tau") o.ransac.tau = toDouble(flag, value);
        else if (verify && flag == "-T") o.ransac.maxIters = toInt(flag, value);
        else if (verify && flag == "-p") o.ransac.p = toDouble(flag, value);
        else if (verify && flag == "-adaptive") o.ransac.adaptive = toInt(flag, value) != 0;
        else if (verify && flag == "-rerank" && (value == "inliers" || value == "matches"))
            o.byMatches = value == "matches";
        else throw std::runtime_error("unknown option " + flag);
    }

    if (o.hpDir.empty() || o.splitFile.empty() || o.outFile.empty())
        throw std::runtime_error("-hp, -split and -o are required");
    if (o.set != "validation" && o.set != "test") throw std::runtime_error("-set must be validation or test");
    if (o.D > 0 && o.mirDir.empty()) throw std::runtime_error("-mir is required when D > 0");
    if (o.vocab < 1 || o.S < 1 || o.D < 0) throw std::runtime_error("-vocab and -S must be positive, -D non-negative");
    if (o.method == Method::None) throw std::runtime_error("choose a method: -exact, -lsh, -hypercube, -ivfflat or -ivfpq");
    if (verify) {
        if ((o.method == Method::IVFFlat || o.method == Method::IVFPQ) && o.ivfflat.kclusters <= 0)
            throw std::runtime_error("-kclusters is required for -ivfflat and -ivfpq");
        if (o.C < 10 || !(o.rho > 0.0 && o.rho < 1.0) || !(o.ransac.tau > 0.0) || o.ransac.maxIters < 1 ||
            !(o.ransac.p > 0.0 && o.ransac.p < 1.0))
            throw std::runtime_error("need C >= 10, 0 < rho < 1, tau > 0, T >= 1 and 0 < p < 1");
    }

    o.lsh.seed = o.cube.seed = o.ivfflat.seed = o.ivfpq.seed = o.seed;
    return o;
}

std::unique_ptr<SearchIndex> makeIndex(const Options& o) {
    switch (o.method) {
        case Method::Exact: return std::make_unique<ExactSearch>();
        case Method::LSH: return std::make_unique<LSHIndex>(o.lsh);
        case Method::Hypercube: return std::make_unique<HypercubeIndex>(o.cube);
        case Method::IVFFlat: return std::make_unique<IVFFlatIndex>(o.ivfflat);
        case Method::IVFPQ: return std::make_unique<IVFPQIndex>(o.ivfpq);
        case Method::None: break;
    }
    throw std::runtime_error("no search method selected");
}