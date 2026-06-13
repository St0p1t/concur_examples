#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>
#include "../9_condvar_futex.cpp"

// Паттерн producer-consumer на FutexConditionVariable.
// В отличие от std::condition_variable, wait() не принимает предикат —
// spurious wakeup нужно обрабатывать вручную через while-цикл.
int main() {
    std::mutex            mtx;
    FutexConditionVariable cv;
    std::queue<int>       q;
    bool                  done = false;

    std::thread producer([&] {
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            {
                std::lock_guard<std::mutex> lk(mtx);
                q.push(i);
                std::cout << "[producer] pushed " << i << "\n";
            }
            cv.notify_one();
        }
        {
            std::lock_guard<std::mutex> lk(mtx);
            done = true;
        }
        cv.notify_all();
    });

    std::thread consumer([&] {
        while (true) {
            std::unique_lock<std::mutex> lk(mtx);
            // Цикл защищает от spurious wakeup (wait() без предиката).
            while (q.empty() && !done)
                cv.wait(lk);

            while (!q.empty()) {
                std::cout << "[consumer] consumed " << q.front() << "\n";
                q.pop();
            }
            if (done) break;
        }
    });

    producer.join();
    consumer.join();
    std::cout << "Done\n";
    return 0;
}
