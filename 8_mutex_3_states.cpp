#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <climits>

static int futex(std::atomic<int>* uaddr, int op, int val) {
    return syscall(SYS_futex, reinterpret_cast<int*>(uaddr), op , val, nullptr, nullptr, 0);
}

class FutexMutex3 {
    std::atomic<int> state{0};
public:
    void lock() {
        int expected = 0;
        if (state.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
            return;
        }

        do {
            if (expected == 2 || state.compare_exchange_strong(expected, 2, std::memory_order_acquire)) {
                futex(&state, FUTEX_WAIT, 2);
            }
            expected = 0;
        } while (!state.compare_exchange_strong(expected, 2, std::memory_order_acquire));
    }

    void unlock() {
        if (state.exchange(0, std::memory_order_release) == 2) {
            futex(&state, FUTEX_WAKE, 1);
        }
    }
};