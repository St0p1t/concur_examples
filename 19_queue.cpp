#include <atomic>

template <typename T>
class MSQueue {
  private:
    struct Node {
        T value;
        std::atomic<Node*> next;
        Node() : next(nullptr) {}
        Node(const T& v) : value(v), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;

  public:
    MSQueue() {
        Node* dummy = new Node();
        head.store(dummy);
        tail.store(dummy);
    }

    void enqueue(const T& value) {
        Node* node = new Node(value);
        while (true) {
            Node* last = tail.load(std::memory_order_acquire);
            Node* next = last->next.load(std::memory_order_acquire);

            if (last == tail.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    if (last->next.compare_exchange_weak(next, node, std::memory_order_release, std::memory_order_relaxed)) {
                        tail.compare_exchange_weak(last, node, std::memory_order_release, std::memory_order_relaxed);
                        return;
                    }
                } else {
                    tail.compare_exchange_weak(last, next, std::memory_order_release, std::memory_order_relaxed);
                }
            }
        }
    }

    bool dequeue(T& result) {
        while (true) {
            Node* first = head.load(std::memory_order_acquire);
            Node* last  = tail.load(std::memory_order_acquire);
            Node* next  = first->next.load(std::memory_order_acquire);

            if (first == head.load(std::memory_order_acquire)) {
                if (first == last) {
                    if (next == nullptr)
                        return false;
                    tail.compare_exchange_weak(last, next, std::memory_order_release, std::memory_order_relaxed);
                } else {
                    result = next->value;
                    if (head.compare_exchange_weak(first, next, std::memory_order_release, std::memory_order_relaxed)) {
                        delete first;
                        return true;
                    }
                }
            }
        }
    }
};