#pragma once

#include <string>
#include <vector>

#include "pipeline/evaluation/metrics.hpp"

// Γράφει το αρχείο εξόδου στη μορφή που ζητάει η εκφώνηση
void writeResults(const std::string& path, const std::string& method, const std::vector<QueryResult>& results);