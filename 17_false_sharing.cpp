#include <atomic>
#include <new>

struct Counters {
    std::atomic<long> a;
    std::atomic<long> b;
};

struct CountersResolved {
    alignas(64) std::atomic<long> a;
    alignas(64) std::atomic<long> b;
};

struct CountersResolvedV2 {
    alignas(std::hardware_destructive_interference_size) std::atomic<long> a;
    alignas(std::hardware_destructive_interference_size) std::atomic<long> b;
};