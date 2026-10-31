#pragma once

#include <cstdint>
#include <string>

#include "../../core/lsh/hypercube.hpp"
#include "../../core/ivf/ivfflat.hpp"
#include "../../core/ivf/ivfpq.hpp"
#include "../../core/lsh/lsh.hpp"

enum class Method { None, Exact, LSH, Hypercube, IVFFlat, IVFPQ };
struct Options {
    std::string hpDir;          // -hp
    std::string mirDir;         // -mir
    std::string splitFile;      // -split
    std::string set;            // -set validation|test
    std::string outFile;        // -o
    std::string cacheDir;       // -cached vocabulary and vectors
    int vocab = 256;            // -vocab, K
    int S = 500;                // -S
    int D = 1000;               // -D
    uint64_t seed = 1;          // -seed
    
    Method method = Method::None;
    LSHParams lsh;
    HypercubeParams cube;
    IVFFlatParams ivfflat;
    IVFPQParams ivfpq;
};

Options parseArgs(int argc, char** argv);

std::string usage();