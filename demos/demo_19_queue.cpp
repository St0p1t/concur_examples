#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include "../19_queue.cpp"

// Lock-free очередь Майкла-Скотта: MPMC (multiple producers, multiple consumers).
// Сначала N потоков параллельно enqueue, затем N потоков параллельно dequeue —
// проверяем, что ни один элемент не потерян и не продублирован.
int main() {
    MSQueue<int> q;
    const int THREADS = 4;
    const int PER     = 5000;
    const int TOTAL   = THREADS * PER;

    // Фаза 1: конкурентный enqueue
    {
        std::vector<std::thread> prods;
        for (int t = 0; t < THREADS; ++t) {
            prods.emplace_back([&, t] {
                for (int i = 0; i < PER; ++i)
                    q.enqueue(t * PER + i);
            });
        }
        for (auto& th : prods) th.join();
    }
    std::cout << "Enqueue: " << TOTAL << " элементов\n";

    // Фаза 2: конкурентный dequeue
    std::atomic<int> dequeued{0};
    {
        std::vector<std::thread> cons;
        for (int t = 0; t < THREADS; ++t) {
            cons.emplace_back([&] {
                int val;
                while (q.dequeue(val))
                    dequeued.fetch_add(1, std::memory_order_relaxed);
            });
        }
        for (auto& th : cons) th.join();
    }

    // Дочистить (потоки могут завершиться, пока ещё есть элементы)
    int val;
    while (q.dequeue(val)) ++dequeued;

    std::cout << "Dequeue: " << dequeued << " элементов\n";
    std::cout << (dequeued == TOTAL ? "OK" : "ОШИБКА — элементы потеряны!") << "\n";
    return 0;
}
