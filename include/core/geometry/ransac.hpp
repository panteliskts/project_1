#pragma once

#include <cstdint>
#include <limits>
#include <vector>

#include "core/geometry/hmgraph.hpp"

struct RansacParams {
    double tau = 3.0;       
    int maxIters = 2000;    
    double p = 0.99;        
    bool adaptive = true;   
    uint64_t seed = 1;
};


struct RansacResult {
    bool valid = false;
    Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
    int inliers = 0;        
    double medianError = std::numeric_limits<double>::infinity();  
};

RansacResult ransacHomography(const std::vector<Point2>& src, const std::vector<Point2>& dst,
                              const RansacParams& params);
                              
int adaptiveIterations(double w, double p, int maxIters);
