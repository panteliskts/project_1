#pragma once

#include <string>
#include <vector>

#include "pipeline/evaluation/metrics.hpp"

void writeResults(const std::string& path, const std::string& method, const std::vector<QueryResult>& results);