#pragma once

#include <vector>

#include "core/common/vec.hpp"


struct Match {
    int q;
    int i;
};


std::vector<Match> ratioMatches(const Dataset& descQ, const Dataset& descI, double rho);
