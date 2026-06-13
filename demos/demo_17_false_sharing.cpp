#include <iostream>
#include <thread>
#include <chrono>
#include "../17_false_sharing.cpp"

// Два потока инкрементируют независимые счётчики a и b.
// Если они лежат в одной cache-line (Counters), процессоры постоянно
// инвалидируют кэши друг друга (cache ping-pong) — MESI protocol.
// CountersResolved / CountersResolvedV2 разносят счётчики по разным линиям.

template <typename C>
long benchmark(const char* name) {
    C counters{};
    counters.a.store(0);
    counters.b.store(0);
    const long ITERS = 50'000'000L;

    auto t0 = std::chrono::high_resolution_clock::now();

    std::thread t1([&] {
        for (long i = 0; i < ITERS; ++i)
            counters.a.fetch_add(1, std::memory_order_relaxed);
    });
    std::thread t2([&] {
        for (long i = 0; i < ITERS; ++i)
            counters.b.fetch_add(1, std::memory_order_relaxed);
    });
    t1.join();
    t2.join();

    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::high_resolution_clock::now() - t0).count();
    std::cout << name << ": " << ms << " ms\n";
    return ms;
}

int main() {
    std::cout << "Cache line: " << std::hardware_destructive_interference_size << " bytes\n\n";

    long slow  = benchmark<Counters>           ("С false sharing    ");
    long fast1 = benchmark<CountersResolved>   ("Без (alignas 64)   ");
    long fast2 = benchmark<CountersResolvedV2> ("Без (hardware hint)");

    std::cout << "\nУскорение (resolved / sharing): "
              << (double)slow / fast1 << "x\n";
    (void)fast2;
    return 0;
}
