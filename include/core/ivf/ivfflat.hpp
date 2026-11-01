#pragma once

#include "core/ivf/ivf.hpp"
#include "core/common/search_index.hpp"

struct IVFFlatParams {
    int kclusters = 0;  // 0 means ceil(sqrt(n))
    int nprobe = 5;     // closest clusters scanned per query
    uint64_t seed = 1;
};


// Μέσα στα nprobe πιο κοντινά clusters, υπολογίζει τις πραγματικές αποστάσεις
class IVFFlatIndex : public SearchIndex {
public:
    explicit IVFFlatIndex(const IVFFlatParams& params);

    std::string name() const override { return "IVFFlat"; }
    void build(const Dataset& data) override;
    std::vector<Neighbor> search(const float* query, int topN) const override;

private:
    IVFFlatParams p_;
    const Dataset* data_ = nullptr;
    InvertedFile ivf_;
};