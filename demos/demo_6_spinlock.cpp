#include <iostream>
#include <thread>
#include <vector>
#include "../6_spinlock.cpp"

// 4 потока инкрементируют общий счётчик 1 000 000 раз каждый.
// Без синхронизации результат был бы меньше 4 000 000 из-за гонки.
int main() {
    SpinLock spin;
    long counter = 0;
    const int  THREADS = 4;
    const long ITERS   = 1'000'000;

    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&] {
            for (long j = 0; j < ITERS; ++j) {
                spin.lock();
                ++counter;
                spin.unlock();
            }
        });
    }
    for (auto& t : threads) t.join();

    const long expected = (long)THREADS * ITERS;
    std::cout << "Ожидалось: " << expected << "\n";
    std::cout << "Получено:  " << counter   << "\n";
    std::cout << (counter == expected ? "OK — гонки нет" : "ОШИБКА — data race!") << "\n";
    return 0;
}
