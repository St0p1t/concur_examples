#include <iostream>
#include <atomic>
#include <mutex>
#include <sstream>
#include "../5_thread_pool.cpp"

int main() {
    const int NUM_TASKS = 12;
    std::atomic<int> completed{0};
    std::mutex print_mtx;

    {
        ThreadPool pool(4);

        for (int i = 0; i < NUM_TASKS; ++i) {
            pool.enqueue([i, &completed, &print_mtx] {
                std::ostringstream oss;
                oss << "Task " << i << " on thread " << std::this_thread::get_id() << "\n";
                {
                    std::lock_guard<std::mutex> lk(print_mtx);
                    std::cout << oss.str();
                }
                ++completed;
            });
        }
    } // деструктор ThreadPool ждёт завершения всех задач

    std::cout << "\nВыполнено задач: " << completed << "/" << NUM_TASKS << "\n";
    return 0;
}
