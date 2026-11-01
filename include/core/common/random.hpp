#pragma once

#include <cmath>
#include <cstdint>
#include <random>

// Generator τυχαίων αριθμών με seed, ώστε να παίρνουμε τα ίδια αποτελέσματα κάθε φορά
class Random {
public:
    explicit Random(uint64_t seed) : gen_(seed) {}

    uint64_t next() { return gen_(); }

    // Τυχαία τιμή ομοιόμορφα κατανεμημένη ανάμεσα στο 0 και το 1
    double uniform() { return double(gen_() >> 11) * 0x1.0p-53; }

    uint64_t below(uint64_t n) { return gen_() % n; }

    // Μέθοδος Box-Muller: κάθε κλήση παράγει δύο κανονικές τιμές, η δεύτερη αποθηκεύεται για την επόμενη φορά
    double normal() {
        if (hasSpare_) {
            hasSpare_ = false;
            return spare_;
        }
        double u1 = 1.0 - uniform();  // (0, 1], keeps log finite
        double u2 = uniform();
        double r = std::sqrt(-2.0 * std::log(u1));
        double theta = 6.283185307179586 * u2;  // 2*pi*u2
        spare_ = r * std::sin(theta);
        hasSpare_ = true;
        return r * std::cos(theta);
    }

private:
    std::mt19937_64 gen_;
    bool hasSpare_ = false;
    double spare_ = 0.0;
};

// Ανακατεύει τα bits (σαν το splitmix64) για καλύτερη τυχαιότητα
inline uint64_t mix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}