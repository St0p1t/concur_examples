#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "../8_mutex_3_states.cpp"

// Мьютекс на трёх состояниях (0=free, 1=locked, 2=locked+waiters).
// Преимущество: если нет спора, unlock() не вызывает FUTEX_WAKE (нет syscall).
// Демонстрация: те же 4 потока × 500 000 инкрементов.
int main() {
    FutexMutex3 mtx;
    long counter = 0;
    const int  THREADS = 4;
    const long ITERS   = 500'000;

    auto t0 = std::chrono::steady_clock::now();

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

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();

    const long expected = (long)THREADS * ITERS;
    std::cout << "Ожидалось: " << expected << "\n";
    std::cout << "Получено:  " << counter   << "\n";
    std::cout << "Время:     " << ms << " мс\n";
    std::cout << (counter == expected ? "OK" : "ОШИБКА") << "\n";
    return 0;
}
