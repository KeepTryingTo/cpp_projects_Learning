
/*
    如果你需要对数组中的某个元素进行复杂的原子操作（例如原子地更新某个元素），
    可以使用 compare_exchange_weak 或 compare_exchange_strong。
*/
#include <atomic>
#include <iostream>
#include <thread>
#include <array>

const int ARRAY_SIZE = 5;
std::atomic<int> atomic_array[ARRAY_SIZE];  // 原子数组

void writer() {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        int expected = i;
        while (!atomic_array[i].compare_exchange_weak(expected, i + 10)) {
            // CAS 失败，重试
        }
        std::cout << "Writer updated index " << i << " to " << i + 10 << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void reader() {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        int value = atomic_array[i].load(std::memory_order_relaxed);  // 原子加载
        std::cout << "Reader read value at index " << i << ": " << value << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        atomic_array[i].store(i, std::memory_order_relaxed);  // 初始化数组
    }

    std::thread t1(writer);
    std::thread t2(reader);

    t1.join();
    t2.join();

    return 0;
}