#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <climits>

static int futex(std::atomic<int>* uaddr, int op, int val) {
    return syscall(SYS_futex, reinterpret_cast<int*>(uaddr), op, val, nullptr, nullptr, 0);
}

class FutexMutex2 {
    std::atomic<int> state{0};
public:
    void lock() {
        int expected = 0;
        while (!state.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
            futex(&state, FUTEX_WAIT, 1);
            expected = 0;
        }
    }

    void unlock() {
        state.store(0, std::memory_order_release);
        futex(&state, FUTEX_WAKE, 1);
    }
};