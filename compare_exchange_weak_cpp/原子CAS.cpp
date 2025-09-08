#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> counter(0);

void increment_counter() {
    int expected;
    do {
        expected = counter.load(); // 读取当前值
    } while (!counter.compare_exchange_weak(expected, expected + 1));
    // 如果counter == expected，则设置为expected + 1；否则expected 更新为counter值
}

void test() {
const int num_threads = 10;
    std::thread threads[num_threads];

    // 创建10个线程
    for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread(increment_counter);
    }
    // 主线程等待子线程执行完成
    for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
    }

    std::cout << "Final counter value: " << counter << std::endl;
    // Final counter value: 10
}

//int main() {
//    test();
//    return 0;
//}