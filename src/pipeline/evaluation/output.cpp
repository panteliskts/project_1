#include "pipeline/evaluation/output.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>

void writeResults(const std::string& path, const std::string& method, const std::vector<QueryResult>& results) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write output file " + path);
    out << std::fixed << std::setprecision(4);

    out << method << "\n";
    for (const QueryResult& r : results) {
        out << "Query: " << r.sequence << "\n";
        for (size_t k = 0; k < 10; ++k) out << "Rank-" << k + 1 << ": " << (k < r.ranked.size() ? r.ranked[k] : "-") << "\n";
        out << "Recall@5: " << r.metrics.recall5 << "\n";
        out << "Recall@10: " << r.metrics.recall10 << "\n";
        out << "AP@10: " << r.metrics.ap10 << "\n\n";
    }

    Summary s = summarize(results);
    out << "Mean Recall@5: " << s.recall5 << "\n";
    out << "Mean Recall@10: " << s.recall10 << "\n";
    out << "mAP@10: " << s.map10 << "\n";
    out << std::setprecision(6) << "Average query time: " << s.ms << "\n";
    if (!out) throw std::runtime_error("error while writing " + path);
}