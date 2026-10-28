#include "pipeline/evaluation/metrics.hpp"

#include <algorithm>

QueryMetrics evaluateRanking(const std::vector<bool>& relevant, int numRelevant) {
    QueryMetrics m;
    int hits = 0;
    for (size_t r = 0; r < relevant.size() && r < 10; ++r) {
        if (!relevant[r]) continue;
        ++hits;
        if (r < 5) m.recall5 += 1.0;
        m.recall10 += 1.0;
        m.ap10 += double(hits) / double(r + 1);
    }
    m.recall5 /= numRelevant;
    m.recall10 /= numRelevant;
    m.ap10 /= numRelevant;
    return m;
}

Summary summarize(const std::vector<QueryResult>& results, const std::function<bool(const QueryResult&)>& keep) {
    Summary s;
    for (const QueryResult& r : results) {
        if (keep && !keep(r)) continue;
        ++s.queries;
        s.recall5 += r.metrics.recall5;
        s.recall10 += r.metrics.recall10;
        s.map10 += r.metrics.ap10;
        s.ms += r.ms;
        s.shortLists += r.ranked.size() < 10;
    }
    if (s.queries > 0) {
        s.recall5 /= s.queries;
        s.recall10 /= s.queries;
        s.map10 /= s.queries;
        s.ms /= s.queries;
    }
    return s;
}