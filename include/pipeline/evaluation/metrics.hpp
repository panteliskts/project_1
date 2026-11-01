#pragma once

#include <functional>
#include <string>
#include <vector>

// Recall@5, Recall@10 και AP@10 ενός query
struct QueryMetrics {
    double recall5 = 0.0;
    double recall10 = 0.0;
    double ap10 = 0.0;
};

// Υπολογίζει τα μετρικά από τη λίστα του ποιες θέσεις της κατάταξης είναι σχετικές
QueryMetrics evaluateRanking(const std::vector<bool>& relevant, int numRelevant = 5);

// Η κατάταξη και τα μετρικά ενός query, μαζί με τον χρόνο αναζήτησης
struct QueryResult {
    std::string sequence;
    std::vector<std::string> ranked;
    QueryMetrics metrics;
    double ms = 0.0;
};

// Μέσοι όροι πάνω σε πολλά queries
struct Summary {
    int queries = 0;
    double recall5 = 0.0, recall10 = 0.0, map10 = 0.0, ms = 0.0;
    int shortLists = 0;
};

// Μέσος όρος των μετρικών, προαιρετικά μόνο στα queries που περνάνε το keep
Summary summarize(const std::vector<QueryResult>& results,
                  const std::function<bool(const QueryResult&)>& keep = nullptr);