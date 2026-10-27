#include "pipeline/cli/cli.hpp"

#include <stdexcept>

namespace {

int toInt(const std::string& flag, const std::string& value) {
    size_t used = 0;
    int v = 0;
    try {
        v = std::stoi(value, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != value.size()) throw std::runtime_error(flag + " expects an integer, got '" + value + "'");
    return v;
}

double toDouble(const std::string& flag, const std::string& value) {
    size_t used = 0;
    double v = 0.0;
    try {
        v = std::stod(value, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != value.size()) throw std::runtime_error(flag + " expects a number, got '" + value + "'");
    return v;
}

void selectMethod(Options& o, Method m) {
    if (o.method != Method::None && o.method != m) throw std::runtime_error("choose exactly one search method");
    o.method = m;
}

}