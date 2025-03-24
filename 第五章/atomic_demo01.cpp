//store and load

/*
writer 线程将值存储到 atomic_value 中，而 reader 线程从 atomic_value 中读取值。
由于使用了 std::memory_order_relaxed，存储和加载操作没有严格的顺序要求，因此读取到的值可能会有所不同。
*/
#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> atomic_value(0);

void writer() {
    for (int i = 0; i < 10; ++i) {
        atomic_value.store(i, std::memory_order_relaxed);  // 使用 relaxed 顺序存储
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void reader() {
    for (int i = 0; i < 10; ++i) {
        int value = atomic_value.load(std::memory_order_relaxed);  // 使用 relaxed 顺序加载
        std::cout << "Read value: " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::thread t1(writer);
    std::thread t2(reader);

    t1.join();
    t2.join();

    return 0;
}