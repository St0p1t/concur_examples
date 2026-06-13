#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include "../18_treiber_stack.cpp"

// Lock-free стек Трайбера: конкурентные push и pop без мьютексов.
// N потоков пушат по PER элементов, затем N потоков конкурентно их достают.
int main() {
    TreiberStack<int> stack;
    const int THREADS = 4;
    const int PER     = 5000;
    const int TOTAL   = THREADS * PER;

    // Конкурентный push
    {
        std::vector<std::thread> pushers;
        for (int t = 0; t < THREADS; ++t) {
            pushers.emplace_back([&, t] {
                for (int i = 0; i < PER; ++i)
                    stack.push(t * PER + i);
            });
        }
        for (auto& th : pushers) th.join();
    }
    std::cout << "Запушено: " << TOTAL << " элементов\n";

    // Конкурентный pop
    std::atomic<int> popped{0};
    {
        std::vector<std::thread> poppers;
        for (int t = 0; t < THREADS; ++t) {
            poppers.emplace_back([&] {
                int val;
                while (stack.pop(val))
                    popped.fetch_add(1, std::memory_order_relaxed);
            });
        }
        for (auto& th : poppers) th.join();
    }

    // Дочистить остаток (если потоки завершились раньше, чем выкачали всё)
    int val;
    while (stack.pop(val)) ++popped;

    std::cout << "Получено:  " << popped << " элементов\n";
    std::cout << (popped == TOTAL ? "OK" : "ОШИБКА — элементы потеряны!") << "\n";
    return 0;
}
