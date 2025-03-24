#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

template <typename T>
class LockFreeStack {
private:
    struct Node {
        T value;
        Node* next;
    };

    std::atomic<Node*> head;

public:
    void push(const T& value) {
        Node* new_node = new Node{value, head.load()};
        while (!head.compare_exchange_strong(new_node->next, new_node)) {
            // 如果失败，new_node->next 会被更新为当前 head，继续重试
        }
    }

    bool pop(T& value) {
        Node* old_head = head.load();
        while (old_head && !head.compare_exchange_strong(old_head, old_head->next)) {
            // 如果失败，old_head 会被更新为当前 head，继续重试
        }
        if (!old_head) {
            return false;  // 栈为空
        }
        value = old_head->value;
        delete old_head;
        return true;
    }
};

int main() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&stack, i]() {
            stack.push(i);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    int value;
    while (stack.pop(value)) {
        std::cout << "Popped value: " << value << "\n";
    }

    return 0;
}