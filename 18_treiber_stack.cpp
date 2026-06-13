#include <atomic>

template <typename T>
class TreiberStack {
  private:
    struct Node {
        T value;
        Node* next;
        Node(const T& v) : value(v), next(nullptr) {}
    };
    std::atomic<Node*> head{nullptr};

  public:
    void push(const T& value) {
        Node* new_node = new Node(value);
        new_node->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(new_node->next, new_node, std::memory_order_release, std::memory_order_relaxed)) {}
    }

    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_acquire);
        while (old_head != nullptr) {
            if (head.compare_exchange_weak(old_head, old_head->next, std::memory_order_acquire, std::memory_order_acquire)) {
                result = old_head->value;
                delete old_head;
                return true;
            }
        }
        return false;
    }
};