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

    // β�巨
    void push(const T& data) {
        Node* new_node = new Node(data);
        new_node->next = head.load();

        // CASѭ��ֱ���ɹ�
        while (!head.compare_exchange_weak(new_node->next, new_node)) {
            // ���head������new_node->next��˵�������߳��޸���head
            // new_node->next�ᱻ����Ϊ��ǰ��head��Ȼ������
        }
        std::cout << "Pushed: " << data << std::endl;
    }

    // ��ͷ���ڵ㿪ʼ����
    bool pop(T& result) {
        Node* old_head = head.load();
        while (old_head &&
            !head.compare_exchange_weak(old_head, old_head->next)) {
            // ���head������old_head��˵�������߳��޸���head
            // old_head�ᱻ����Ϊ��ǰ��head��Ȼ������
        }

        if (!old_head) {
            std::cout << "Pop failed - stack empty" << std::endl;
            return false; // ջΪ��
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
        while (pop(temp)); // �������нڵ�
    }
};

// ���Ժ��� - �������߳�
void producer(LockFreeStack<int>& stack, int thread_id, int num_operations) {
    // �������������num_operations����
    for (int i = 0; i < num_operations; ++i) {
        int value = thread_id * 100 + i;
        stack.push(value);
        // ����˯��10ms����Ҫ��Ϊ���������߽�������
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// ���Ժ��� - �������̣߳����ջΪ�վ͵ȴ���������������
void consumer(LockFreeStack<int>& stack, int thread_id, int num_operations) {
    int value;
    int successful_pops = 0;

    while (successful_pops < num_operations) {
        // ���ջ��Ϊ�վͼ������ѣ�ֱ��num)operationsΪֹ
        if (stack.pop(value)) {
            successful_pops++;
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        else {
            // ջΪ�գ��Ե�����
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

    std::cout << "=== ��ʼ�����̰߳�ȫջ ===" << std::endl;

    // �����������߳�
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back(producer, std::ref(stack), i + 1, operations_per_thread);
    }

    // �����������߳�
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back(consumer, std::ref(stack), i + 1, operations_per_thread);
    }

    // �ȴ��������������
    for (auto& t : producers) {
        t.join();
    }
    std::cout << "�����������߳������" << std::endl;

    // �ȴ��������������
    for (auto& t : consumers) {
        t.join();
    }
    std::cout << "�����������߳������" << std::endl;

    // ����ջ�Ƿ�Ϊ��
    std::cout << "ջ�Ƿ�Ϊ��: " << (stack.isEmpty() ? "��" : "��") << std::endl;

    // ������ԣ����̲߳���
    std::cout << "\n=== ��ʼ���̲߳��� ===" << std::endl;

    // ѹ��һЩ����
    stack.push(999);
    stack.push(888);
    stack.push(777);

    // ��������
    int value;
    while (stack.pop(value)) {
        std::cout << "���̵߳���: " << value << std::endl;
    }

    // ���Կ�ջ����
    if (!stack.pop(value)) {
        std::cout << "��ջ�������Գɹ�" << std::endl;
    }

    std::cout << "=== ������� ===" << std::endl;
}

void simple_test() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;

    const int num_threads = 4;
    const int operations_per_thread = 3;

    for (int i = 0; i < num_threads; ++i) {
        // ʹ��lambda���ʽʵ�ֺ���
        threads.emplace_back([&stack, i]() {
            // ÿ���߳���push��pop
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

    std::cout << "������ɣ�ջ�Ƿ�Ϊ��: " << (stack.isEmpty() ? "��" : "��") << std::endl;
}


int main() {
    
    //simple_test1();
    simple_test();
    return 0;
}