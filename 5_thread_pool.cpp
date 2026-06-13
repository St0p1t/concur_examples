#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
  private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mtx;
    std::condition_variable cv;
    bool stop = false;

  public:
    explicit ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i)
            workers.emplace_back([this] { worker_loop(); });
    }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one();
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mtx);
            stop = true;
        }
        cv.notify_all();
        for (std::thread& w : workers)
            w.join();
    }

  private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mtx);
                cv.wait(lock, [this] { return stop || !tasks.empty(); });

                if (stop && tasks.empty())
                    return;

                task = std::move(tasks.front());
                tasks.pop();
            }

            task();
        }
    }
};