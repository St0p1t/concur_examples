#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "../4_semaphore.cpp"

// Симулируем пул из 3 соединений, к которому обращаются 8 воркеров одновременно.
// Семафор ограничивает число одновременно работающих — не более POOL_SIZE.
int main() {
    const int POOL_SIZE = 3;
    const int WORKERS   = 8;

    counting_semaphore<POOL_SIZE> sem(POOL_SIZE);
    std::vector<std::thread> threads;

    for (int i = 0; i < WORKERS; ++i) {
        threads.emplace_back([&sem, i] {
            sem.acquire();
            std::cout << "Worker " << i << " >>> acquired  (active: ?)\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << "Worker " << i << " <<< released\n";
            sem.release();
        });
    }

    for (auto& t : threads) t.join();
    std::cout << "All workers done\n";
    return 0;
}
