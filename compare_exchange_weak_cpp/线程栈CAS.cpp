#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <chrono>

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& data) : data(data), next(nullptr) {}
    };

    std::atomic<Node*> head;

public:
    LockFreeStack() : head(nullptr) {}

    // 尾插法
    void push(const T& data) {
        Node* new_node = new Node(data);
        new_node->next = head.load();

        // CAS循环直到成功
        while (!head.compare_exchange_weak(new_node->next, new_node)) {
            // 如果head不等于new_node->next，说明其他线程修改了head
            // new_node->next会被更新为当前的head，然后重试
        }
        std::cout << "Pushed: " << data << std::endl;
    }

    // 从头部节点开始弹出
    bool pop(T& result) {
        Node* old_head = head.load();
        while (old_head &&
            !head.compare_exchange_weak(old_head, old_head->next)) {
            // 如果head不等于old_head，说明其他线程修改了head
            // old_head会被更新为当前的head，然后重试
        }

        if (!old_head) {
            std::cout << "Pop failed - stack empty" << std::endl;
            return false; // 栈为空
        }

        result = old_head->data;
        std::cout << "Popped: " << result << std::endl;
        delete old_head;
        return true;
    }

    bool isEmpty() const {
        return head.load() == nullptr;
    }

    ~LockFreeStack() {
        T temp;
        while (pop(temp)); // 清理所有节点
    }
};

// 测试函数 - 生产者线程
void producer(LockFreeStack<int>& stack, int thread_id, int num_operations) {
    // 消费者最多生产num_operations数据
    for (int i = 0; i < num_operations; ++i) {
        int value = thread_id * 100 + i;
        stack.push(value);
        // 这里睡眠10ms，主要是为了让消费者进行消费
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// 测试函数 - 消费者线程，如果栈为空就等待生产者生产数据
void consumer(LockFreeStack<int>& stack, int thread_id, int num_operations) {
    int value;
    int successful_pops = 0;

    while (successful_pops < num_operations) {
        // 如果栈不为空就继续消费，直到num)operations为止
        if (stack.pop(value)) {
            successful_pops++;
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        else {
            // 栈为空，稍等再试
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

void simple_test1() {
    LockFreeStack<int> stack;
    const int num_producers = 3;
    const int num_consumers = 2;
    const int operations_per_thread = 5;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    std::cout << "=== 开始测试线程安全栈 ===" << std::endl;

    // 创建生产者线程
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back(producer, std::ref(stack), i + 1, operations_per_thread);
    }

    // 创建消费者线程
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back(consumer, std::ref(stack), i + 1, operations_per_thread);
    }

    // 等待所有生产者完成
    for (auto& t : producers) {
        t.join();
    }
    std::cout << "所有生产者线程已完成" << std::endl;

    // 等待所有消费者完成
    for (auto& t : consumers) {
        t.join();
    }
    std::cout << "所有消费者线程已完成" << std::endl;

    // 测试栈是否为空
    std::cout << "栈是否为空: " << (stack.isEmpty() ? "是" : "否") << std::endl;

    // 额外测试：单线程操作
    std::cout << "\n=== 开始单线程测试 ===" << std::endl;

    // 压入一些数据
    stack.push(999);
    stack.push(888);
    stack.push(777);

    // 弹出数据
    int value;
    while (stack.pop(value)) {
        std::cout << "主线程弹出: " << value << std::endl;
    }

    // 测试空栈弹出
    if (!stack.pop(value)) {
        std::cout << "空栈弹出测试成功" << std::endl;
    }

    std::cout << "=== 测试完成 ===" << std::endl;
}

void simple_test() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;

    const int num_threads = 4;
    const int operations_per_thread = 3;

    for (int i = 0; i < num_threads; ++i) {
        // 使用lambda表达式实现函数
        threads.emplace_back([&stack, i]() {
            // 每个线程先push再pop
            for (int j = 0; j < 3; ++j) {
                int value = i * 10 + j;
                stack.push(value);

                int popped;
                if (stack.pop(popped)) {
                    std::cout << "Thread " << i << " popped: " << popped << std::endl;
                }
            }
            });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "测试完成，栈是否为空: " << (stack.isEmpty() ? "是" : "否") << std::endl;
}


int main() {
    
    //simple_test1();
    simple_test();
    return 0;
}