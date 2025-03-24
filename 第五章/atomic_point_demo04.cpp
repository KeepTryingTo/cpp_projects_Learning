#include <atomic>
#include <iostream>
#include <thread>
#include <array>

const int ARRAY_SIZE = 5;
using ArrayType = std::array<int, ARRAY_SIZE>;

std::atomic<ArrayType*> atomic_array_ptr;  // 原子指针，指向数组

void writer() {
    auto new_array = new ArrayType{1, 2, 3, 4, 5};
    atomic_array_ptr.store(new_array, std::memory_order_release);  // 原子存储指针
    std::cout << "Writer stored a new array.\n";
}

void reader() {
    ArrayType* array_ptr = atomic_array_ptr.load(std::memory_order_acquire);  // 原子加载指针
    if (array_ptr) {
        std::cout << "Reader loaded array: ";
        for (int value : *array_ptr) {
            std::cout << value << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    ArrayType* initial_array = new ArrayType{0, 0, 0, 0, 0};
    atomic_array_ptr.store(initial_array, std::memory_order_relaxed);

    std::thread t1(writer);
    std::thread t2(reader);

    t1.join();
    t2.join();

    // 清理内存
    delete atomic_array_ptr.load();
    return 0;
}