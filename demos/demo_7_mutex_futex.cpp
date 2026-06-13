#include <iostream>
#include <thread>
#include <vector>
#include "../7_mutex_futex.cpp"

// Мьютекс на двух состояниях (futex): 4 потока инкрементируют счётчик.
// При высоком споре поток уходит в FUTEX_WAIT вместо busy-spin.
int main() {
    FutexMutex2 mtx;
    long counter = 0;
    const int  THREADS = 4;
    const long ITERS   = 500'000;

    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&] {
            for (long j = 0; j < ITERS; ++j) {
                mtx.lock();
                ++counter;
                mtx.unlock();
            }
        });
    }
    for (auto& t : threads) t.join();

    const long expected = (long)THREADS * ITERS;
    std::cout << "Ожидалось: " << expected << "\n";
    std::cout << "Получено:  " << counter   << "\n";
    std::cout << (counter == expected ? "OK" : "ОШИБКА") << "\n";
    return 0;
}
