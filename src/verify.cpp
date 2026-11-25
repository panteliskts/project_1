#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <tuple>

#include "core/common/random.hpp"
#include "core/geometry/matching.hpp"
#include "core/geometry/ransac.hpp"
#include "pipeline/cli/cli.hpp"
#include "pipeline/dataset/database.hpp"
#include "pipeline/dataset/split.hpp"
#include "pipeline/evaluation/metrics.hpp"
#include "pipeline/features/keypoints.hpp"

namespace {

using Clock = std::chrono::steady_clock;

double msSince(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

struct Check {
    int base;        
    int rank;          
    int matches;       
    RansacResult ransac;
};

struct HomographyStats {
    int valid = 0;
    int accurate = 0;    
    double errorSum = 0.0;
};

} 

int main(int argc, char** argv) {
    if (argc == 1) {
        std::fputs(usage(true).c_str(), stderr);
        return 1;
    }
    try {
        Options opt = parseArgs(argc, argv, true);
        Collections col = buildCollections(opt, readSplit(opt.splitFile));
        std::unique_ptr<SearchIndex> index = makeIndex(opt);
        index->build(col.base);

        const int nq = col.queries.n;
        std::vector<std::vector<Neighbor>> candidates(nq);
        std::vector<QueryResult> before(nq), after(nq);  
        for (int q = 0; q < nq; ++q) {
            Clock::time_point t0 = Clock::now();
            candidates[q] = index->search(col.queries.row(q), opt.C);
            before[q].ms = msSince(t0);
        }

        std::vector<std::string> paths = col.queryPaths;
        std::vector<int> featureOf(col.base.n, -1);
        for (const std::vector<Neighbor>& list : candidates)
            for (const Neighbor& nb : list)
                if (featureOf[nb.id] < 0) {
                    featureOf[nb.id] = int(paths.size());
                    paths.push_back(col.basePaths[nb.id]);
                }
        std::fprintf(stderr, "Loading the SIFT keypoints of %zu images\n", paths.size());
        std::vector<ImageFeatures> features = loadOrExtractFeatures(paths, opt.cacheDir);

        std::string method = index->name() + (opt.byMatches ? " (re-ranked by SIFT matches)" : "");
        std::ofstream out(opt.outFile);
        if (!out) throw std::runtime_error("cannot write output file " + opt.outFile);
        out << std::fixed << std::setprecision(4) << method << "\n";
            }
        }