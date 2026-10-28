#pragma once

#include <string>
#include <vector>

#include "core/common/vec.hpp"
#include "pipeline/cli/cli.hpp"
#include "pipeline/dataset/split.hpp"

struct Collections {
    std::vector<std::string> querySeqs;  
    Dataset queries;

    std::vector<std::string> baseIds;    
    std::vector<int> baseQuery;          
    Dataset base;                        

    int mirflickrCount = 0;
};


Collections buildCollections(const Options& o, const std::vector<SplitEntry>& split);