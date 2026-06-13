#include <atomic>

class SpinLock {
    std::atomic<bool> locked_{false};

public:
    void lock() {
        while (locked_.exchange(true, std::memory_order_acquire)) {}
    }

    void unlock() {
        locked_.store(false, std::memory_order_release);
    }
};