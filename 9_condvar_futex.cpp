#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <climits>
#include <mutex>

static int futex(std::atomic<int>* uaddr, int op, int val) {
    return syscall(SYS_futex, reinterpret_cast<int*>(uaddr), op , val, nullptr, nullptr, 0);
}

class FutexConditionVariable {
    std::atomic<int> seq{0};
public:
    void wait(std::unique_lock<std::mutex>& lock) {
        int old = seq.load(std::memory_order_relaxed);
        lock.unlock();
        futex(&seq, FUTEX_WAIT, old);
        lock.lock();
    }

    void notify_one() {
        seq.fetch_add(1, std::memory_order_relaxed);
        futex(&seq, FUTEX_WAKE, 1);
    }

    void notify_all() {
        seq.fetch_add(1, std::memory_order_relaxed);
        futex(&seq, FUTEX_WAKE, INT_MAX);
    }
};