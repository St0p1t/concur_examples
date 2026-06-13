#include <mutex>
#include <condition_variable>


template <int MaxValue>
class counting_semaphore {
  private:
    std::mutex m_;
    std::condition_variable cv_;
    int count_;

  public:
    explicit counting_semaphore(int initial) : count_(initial) {}

    counting_semaphore(const counting_semaphore&) = delete;
    counting_semaphore& operator=(const counting_semaphore&) = delete;

    void acquire() {
        std::unique_lock<std::mutex> lock(m_);
        cv_.wait(lock, [this] { return count_ > 0; });
        --count_;
    }

    void release() {
        {
            std::lock_guard<std::mutex> lock(m_);
            ++count_;
        }
        cv_.notify_one();
    }
};