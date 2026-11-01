#include "core/exact/exact.hpp"

// Υπολογίζει την απόσταση του query από κάθε σημείο της βάσης και κρατά τα topN πλησιέστερα
std::vector<Neighbor> ExactSearch::search(const float* query, int topN) const {
    std::vector<Neighbor> cands(data_->n);
    for (int i = 0; i < data_->n; ++i)
        cands[i] = {i, squaredL2(query, data_->row(i), data_->dim)};
    return topNeighbors(cands, topN);
}