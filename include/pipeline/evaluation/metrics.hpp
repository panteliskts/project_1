#pragma once

#include <functional>
#include <string>
#include <vector>

struct QueryMetrics {
    double recall5 = 0.0;
    double recall10 = 0.0;
    double ap10 = 0.0;
};

QueryMetrics evaluateRanking(const std::vector<bool>& relevant, int numRelevant = 5);

struct QueryResult {
    std::string sequence;             
    std::vector<std::string> ranked;  
    QueryMetrics metrics;
    double ms = 0.0;                  
};

struct Summary {
    int queries = 0;
    double recall5 = 0.0, recall10 = 0.0, map10 = 0.0, ms = 0.0;
    int shortLists = 0;  
};

Summary summarize(const std::vector<QueryResult>& results,
                  const std::function<bool(const QueryResult&)>& keep = nullptr);