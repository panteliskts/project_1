#pragma once

#include <cstdint>

#include "core/common/search_index.hpp"
#include "core/ivf/ivf.hpp"

// Παράμετροι IVFPQ: M υποδιανύσματα, το nbits λέει πόσα υποκέντρα έχει το καθένα
// (η διάσταση dim πρέπει να διαιρείται ακριβώς με το M)
struct IVFPQParams {
    int kclusters = 0;
    int nprobe = 5;
    int M = 16;
    int nbits = 8;
    uint64_t seed = 1;
};


// Συνδυάζει IVF και Product Quantization, κρατά συμπιεσμένους κωδικούς αντί για πραγματικά διανύσματα
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
    std::vector<Dataset> codebooks_;  // M codebooks, ένα ανά υποδιάνυσμα
    std::vector<uint16_t> codes_;     // ο κωδικός κάθε σημείου ανά υποδιάνυσμα
};