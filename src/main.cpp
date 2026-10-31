#include <chrono>
#include <cstdio>
#include <memory>
#include <stdexcept>

#include "core/exact/exact.hpp"
#include "core/ivf/ivfflat.hpp"
#include "core/ivf/ivfpq.hpp"
#include "core/lsh/hypercube.hpp"
#include "core/lsh/lsh.hpp"
#include "pipeline/cli/cli.hpp"
#include "pipeline/dataset/database.hpp"
#include "pipeline/dataset/split.hpp"
#include "pipeline/evaluation/metrics.hpp"
#include "pipeline/evaluation/output.hpp"

namespace {

using Clock = std::chrono::steady_clock;

double msSince(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
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

void printSummary(const char* label, const Summary& s) {
    std::printf("  %-13s queries=%2d  R@5=%.4f  R@10=%.4f  mAP@10=%.4f  time=%.4f ms  (<10 results: %d)\n", label,
                s.queries, s.recall5, s.recall10, s.map10, s.ms, s.shortLists);
}

} 

int main(int argc, char** argv) {
    if (argc == 1) {
        std::fputs(usage().c_str(), stderr);
        return 1;
    }
    
}