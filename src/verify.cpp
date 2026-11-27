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

        std::vector<HomographyStats> stats(nq);
        for (int q = 0; q < nq; ++q) {
            const ImageFeatures& qf = features[q];
            Clock::time_point t0 = Clock::now();
            std::vector<Check> checks;
            for (int r = 0; r < int(candidates[q].size()); ++r) {
                int base = candidates[q][r].id;
                const ImageFeatures& f = features[featureOf[base]];
                std::vector<Match> matches = ratioMatches(qf.desc, f.desc, opt.rho);
                std::vector<Point2> src, dst;
                for (const Match& m : matches) {
                    src.push_back(qf.points[m.q]);
                    dst.push_back(f.points[m.i]);
                }
                RansacParams params = opt.ransac;
                params.seed = mix64(opt.seed) ^ mix64((uint64_t(q) << 32) | uint64_t(base));
                checks.push_back({base, r, int(matches.size()), ransacHomography(src, dst, params)});
            }

            auto key = [&](const Check& c) {
                return std::make_tuple(-(opt.byMatches ? c.matches : c.ransac.inliers), c.ransac.medianError, c.rank);
            };
            std::sort(checks.begin(), checks.end(), [&](const Check& a, const Check& b) { return key(a) < key(b); });
            after[q].ms = msSince(t0);

            const std::string& seq = col.querySeqs[q];
            out << "Query: " << seq << "\nCandidates: " << candidates[q].size() << "\n";
            std::vector<bool> relBefore, relAfter;
            for (size_t k = 0; k < 10; ++k) {
                bool has = k < candidates[q].size();
                out << "Before-Rank-" << k + 1 << ": " << (has ? col.baseIds[candidates[q][k].id] : "-") << "\n";
                if (has) relBefore.push_back(col.baseQuery[candidates[q][k].id] == q);
            }
            for (size_t k = 0; k < 10; ++k) {
                bool has = k < checks.size();
                const RansacResult& r = has ? checks[k].ransac : RansacResult();
                out << "After-Rank-" << k + 1 << ": " << (has ? col.baseIds[checks[k].base] : "-") << "\n";
                out << "After-Inliers-" << k + 1 << ": " << (has ? std::to_string(r.inliers) : "-") << "\n";
                out << "After-MedianError-" << k + 1 << ": ";
                if (has && r.valid) out << r.medianError << "\n";
                else out << (has ? "NA" : "-") << "\n";
                if (has) relAfter.push_back(col.baseQuery[checks[k].base] == q);
            }
            before[q].sequence = after[q].sequence = seq;
            before[q].metrics = evaluateRanking(relBefore);
            after[q].metrics = evaluateRanking(relAfter);
            for (auto [label, m] : {std::make_pair("Before", before[q].metrics),
                                    std::make_pair("After", after[q].metrics)})
                out << label << "-Recall@5: " << m.recall5 << "\n" << label << "-Recall@10: " << m.recall10 << "\n"
                    << label << "-AP@10: " << m.ap10 << "\n";
            out << "\n";

            for (const Check& c : checks) {
                if (col.baseQuery[c.base] != q || !c.ransac.valid) continue;
                const std::string& id = col.baseIds[c.base];
                std::ifstream in(opt.hpDir + "/" + seq + "/H_1_" + id.substr(id.rfind('/') + 1));
                Eigen::Matrix3d Hgt;
                for (int i = 0; i < 9; ++i)
                    if (!(in >> Hgt(i / 3, i % 3))) throw std::runtime_error("cannot read the homography of " + id);
                double err = cornerError(c.ransac.H, Hgt, qf.width, qf.height);
                if (!std::isfinite(err)) continue;  
                ++stats[q].valid;
                stats[q].accurate += err < 5.0;
                stats[q].errorSum += err;
            }
        }

        std::printf("%s on %s set: base n=%d (%d HPatches + %d MIRFlickr), C=%d, rho=%.2f, tau=%.2f, T=%d, p=%.3f, "
                    "adaptive=%d\n", method.c_str(), opt.set.c_str(), col.base.n, col.base.n - col.mirflickrCount,
                    col.mirflickrCount, opt.C, opt.rho, opt.ransac.tau, opt.ransac.maxIters, opt.ransac.p,
                    int(opt.ransac.adaptive));

        for (const char* part : {"all", "illumination", "viewpoint"}) {
            auto keep = [part](const QueryResult& r) {
                return part[0] == 'a' || isIllumination(r.sequence) == (part[0] == 'i');
            };
            HomographyStats h;
            for (int q = 0; q < nq; ++q)
                if (keep(before[q])) {
                    h.valid += stats[q].valid;
                    h.accurate += stats[q].accurate;
                    h.errorSum += stats[q].errorSum;
                }
            Summary b = summarize(before, keep), a = summarize(after, keep);
            double validRate = h.valid / (5.0 * b.queries), accurateRate = h.accurate / (5.0 * b.queries);
            double cornerErr = h.valid > 0 ? h.errorSum / h.valid : std::nan("");
            std::printf("  %-13s queries=%2d  mAP@10 %.4f -> %.4f  R@5 %.4f -> %.4f  R@10 %.4f -> %.4f\n", part,
                        b.queries, b.map10, a.map10, b.recall5, a.recall5, b.recall10, a.recall10);
            std::printf("  %-13s valid H %.4f  accurate(<5px) %.4f  corner error %.4f px  time %.3f + %.3f = %.3f ms\n",
                        "", validRate, accurateRate, cornerErr, b.ms, a.ms, b.ms + a.ms);
            if (part[0] != 'a') continue;
            out << "MeanBeforeRecall@5: " << b.recall5 << "\nMeanBeforeRecall@10: " << b.recall10
                << "\nMeanBeforeMAP@10: " << b.map10 << "\nMeanAfterRecall@5: " << a.recall5
                << "\nMeanAfterRecall@10: " << a.recall10 << "\nMeanAfterMAP@10: " << a.map10 << std::setprecision(6)
                << "\nAverageCandidateTimeMs: " << b.ms << "\nAverageVerificationTimeMs: " << a.ms
                << "\nAverageTotalTimeMs: " << b.ms + a.ms << std::setprecision(4) << "\nValidHomographyRate: "
                << validRate << "\nAccurateHomographyRate5px: " << accurateRate << "\nMeanCornerErrorPx: ";
            if (std::isnan(cornerErr)) out << "NA\n";
            else out << cornerErr << "\n";
            if (!out) throw std::runtime_error("error while writing " + opt.outFile);
        }
        std::printf("Results written to %s\n", opt.outFile.c_str());
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n\n%s", e.what(), usage(true).c_str());
        return 1;
    }
    return 0;
}