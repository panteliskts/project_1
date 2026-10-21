#pragma once

#include <string>
#include <vector>

struct SplitEntry {
    std::string sequence;  // directory name
    std::string part;     
};

// Reads lines of the form "<sequence> <train|validation|test>".
std::vector<SplitEntry> readSplit(const std::string& path);

inline bool isIllumination(const std::string& sequence) { return sequence.rfind("i_", 0) == 0; }