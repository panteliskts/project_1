#pragma once

#include <cstdint>

#include "core/common/search_index.hpp"
#include "core/ivf/ivf.hpp"

struct IVFPQParams {
    int kclusters = 0;  
    int nprobe = 5;     
    int M = 16;         
    int nbits = 8;     
    uint64_t seed = 1;
};


class IVFPQIndex : public SearchIndex {
public:
    explicit IVFPQIndex(const IVFPQParams& params);

    std::string name() const override { return "IVFPQ"; }
    void build(const Dataset& data) override;
    std::vector<Neighbor> search(const float* query, int topN) const override;

private:
    IVFPQParams p_;
    int dim_ = 0;
    int subDim_ = 0;
    int ksub_ = 0;                    
    InvertedFile ivf_;
    std::vector<Dataset> codebooks_;  
    std::vector<uint16_t> codes_;     
};