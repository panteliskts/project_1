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

// Φτιάχνει τη μέθοδο αναζήτησης που επιλέχθηκε, με τις παραμέτρους της
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

// Τυπώνει μία γραμμή με τους μέσους όρους στο stderr
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
    try {
        Options opt = parseArgs(argc, argv);
        std::vector<SplitEntry> split = readSplit(opt.splitFile);
        Collections col = buildCollections(opt, split);

        std::unique_ptr<SearchIndex> index = makeIndex(opt);
        Clock::time_point t0 = Clock::now();
        index->build(col.base);
        double buildMs = msSince(t0);

        // Χτίζει το ευρετήριο μία φορά και μετά τρέχει κάθε query ξεχωριστά,
        // μετρώντας μόνο τον χρόνο της search
        std::vector<QueryResult> results;
        for (int q = 0; q < col.queries.n; ++q) {
            t0 = Clock::now();
            std::vector<Neighbor> found = index->search(col.queries.row(q), 10);
            double ms = msSince(t0);

            QueryResult r;
            r.sequence = col.querySeqs[q];
            r.ms = ms;
            std::vector<bool> relevant;
            for (const Neighbor& nb : found) {
                r.ranked.push_back(col.baseIds[nb.id]);
                // Σχετική είναι η εικόνα της βάσης που ανήκει στην ίδια ακολουθία HPatches με το query
                relevant.push_back(col.baseQuery[nb.id] == q);
            }
            r.metrics = evaluateRanking(relevant);
            results.push_back(r);
        }
        writeResults(opt.outFile, index->name(), results);

        std::printf("%s on %s set: base n=%d (%d HPatches + %d MIRFlickr), K=%d, index build %.1f ms\n",
                    index->name().c_str(), opt.set.c_str(), col.base.n, col.base.n - col.mirflickrCount,
                    col.mirflickrCount, col.base.dim, buildMs);
        printSummary("all", summarize(results));
        printSummary("illumination", summarize(results, [](const QueryResult& r) { return isIllumination(r.sequence); }));
        printSummary("viewpoint", summarize(results, [](const QueryResult& r) { return !isIllumination(r.sequence); }));
        std::printf("Results written to %s\n", opt.outFile.c_str());
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n\n%s", e.what(), usage().c_str());
        return 1;
    }
    return 0;
}