#pragma once

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

template <class Fn>
void parallelFor(int begin, int end, Fn fn, int maxThreads = 0) {
    if (end <= begin) return;
    int threads = int(std::max(1u, std::thread::hardware_concurrency()));
    if (maxThreads > 0) threads = std::min(threads, maxThreads);
    threads = std::min(threads, end - begin);
    if (threads == 1) {
        for (int i = begin; i < end; ++i) fn(i);
        return;
    }
    std::atomic<int> nextItem(begin);
    std::vector<std::thread> pool;
    for (int t = 0; t < threads; ++t) {
        pool.emplace_back([&] {
            for (int i = nextItem++; i < end; i = nextItem++) fn(i);
        });
    }
    for (std::thread& th : pool) th.join();
}