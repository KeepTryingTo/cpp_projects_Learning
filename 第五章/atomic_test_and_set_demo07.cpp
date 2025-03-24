/*
    test_and_set 是 std::atomic_flag 类的一个成员函数，用于原子地执行以下操作：
    将标志位设置为 true。
    返回标志位之前的值。

    test_and_set 通常用于实现简单的同步原语，例如自旋锁（spinlock）。
    自旋锁是一种忙等待的锁，线程会不断检查锁的状态，直到锁被释放。
*/

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::atomic_flag lock = ATOMIC_FLAG_INIT;  // 初始化标志位为 false
int shared_data = 0;  // 共享数据

void increment(int id) {
    for (int i = 0; i < 1000; ++i) {
        // 自旋锁：忙等待直到获取锁
        while (lock.test_and_set(std::memory_order_acquire)) {
            // 忙等待
        }

        // 临界区：操作共享数据
        ++shared_data;

        // 释放锁
        lock.clear(std::memory_order_release);
    }
    std::cout << "Thread " << id << " finished.\n";
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(increment, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Final value of shared_data: " << shared_data << "\n";
    return 0;
}