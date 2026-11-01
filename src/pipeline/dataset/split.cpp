#include "pipeline/dataset/split.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

// Διαβάζει το αρχείο split γραμμή προς γραμμή: μία ακολουθία και το σύνολό της ανά γραμμή
std::vector<SplitEntry> readSplit(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open split file " + path);

    std::vector<SplitEntry> entries;
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        std::istringstream fields(line);
        SplitEntry e;
        if (!(fields >> e.sequence)) continue;  
        if (!(fields >> e.part) || (e.part != "train" && e.part != "validation" && e.part != "test"))
            throw std::runtime_error(path + ":" + std::to_string(lineNo) + ": expected '<sequence> train|validation|test'");
        entries.push_back(e);
    }
    return entries;
}