#pragma once

#include <string>
#include <vector>

#include "core/common/vec.hpp"
#include "pipeline/cli/cli.hpp"
#include "pipeline/dataset/split.hpp"

// Τα queries και η βάση, έτοιμα για αναζήτηση
struct Collections {
    std::vector<std::string> querySeqs;  
    Dataset queries;

    std::vector<std::string> baseIds;    
    std::vector<int> baseQuery;          
    Dataset base;                        

    int mirflickrCount = 0;
};


// Φτιάχνει το λεξιλόγιο, τα διανύσματα και τα Collections για το ζητούμενο σύνολο
Collections buildCollections(const Options& o, const std::vector<SplitEntry>& split);