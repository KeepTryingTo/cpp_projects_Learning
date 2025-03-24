#include <atomic>
#include <iostream>
#include <thread>
#include <array>
#include <mutex>

const int ARRAY_SIZE = 5;
std::array<int, ARRAY_SIZE> shared_array;
std::mutex array_mutex;  // 互斥锁，用于保护数组

void writer() {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        std::lock_guard<std::mutex> lock(array_mutex);  // 加锁
        shared_array[i] = i;  // 修改数组
        std::cout << "Writer updated index " << i << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void reader() {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        std::lock_guard<std::mutex> lock(array_mutex);  // 加锁
        int value = shared_array[i];  // 读取数组
        std::cout << "Reader read value at index " << i << ": " << value << "\n";
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