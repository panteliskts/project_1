#pragma once

#include <cstdint>
#include <string>

#include "../core/hypercube.hpp"
#include "../core/ivfflat.hpp"
#include "../core/ivfpq.hpp"
#include "../core/lsh.hpp"

enum class Method { None, Exact, LSH, Hypercube, IVFFlat, IVFPQ };

struct Options {
    std::string hpDir;         
    std::string mirDir;         
    std::string splitFile;      
    std::string set;            
    std::string outFile;        
    std::string cacheDir;       
    int vocab = 256;            
    int S = 500;               
    int D = 1000;               
    uint64_t seed = 1;          

    Method method = Method::None;
    LSHParams lsh;
    HypercubeParams cube;
    IVFFlatParams ivfflat;
    IVFPQParams ivfpq;
};

Options parseArgs(int argc, char** argv);

std::string usage();