#include <iostream>
#include <atomic>
#include <thread>

class SpinLock {
private:
    std::atomic<bool> locked{ false };

public:
    void lock() {
        bool expected = false;
        while (!locked.compare_exchange_weak(expected, true,
            std::memory_order_acquire, std::memory_order_relaxed)) {
            // 重置expected值，继续循环，从而达到自旋锁的目的
            expected = false; 
            // 可选: 加入等待策略减少CPU占用
        }
    }

    void unlock() {
        locked.store(false, std::memory_order_release);
    }
};

SpinLock spin_lock;
int shared_data = 0;

void worker(int id) {
    spin_lock.lock();
    std::cout << "Thread " << id << " acquired lock" << std::endl;
    shared_data++;
    std::cout << "Thread " << id << " released lock" << std::endl;
    spin_lock.unlock();
}

//int main() {
//    std::thread t1(worker, 1);
//    std::thread t2(worker, 2);
//
//    t1.join();
//    t2.join();
//
//    std::cout << "Final shared_data: " << shared_data << std::endl;
//    return 0;
//}
/*
1. 初始状态
    locked 初始值为 false（未锁定）
    expected 初始值为 false

2. 第一个线程进入lock()
    compare_exchange_weak 检查：locked == expected（false == false）
    条件成立，于是将 locked 设置为 true，函数返回 true
    while循环条件 !true 为 false，退出循环，线程获得锁

3. 第二个线程进入lock()
    此时 locked 已经是 true
    compare_exchange_weak 检查：locked == expected（true == false）
    条件不成立，函数返回 false

    while循环条件 !false 为 true，继续循环
    重置 expected = false，继续尝试

4. 第一个线程调用unlock()
    locked.store(false) 将锁释放

5. 第二个线程再次尝试
    compare_exchange_weak 检查：locked == expected（false == false）
    条件成立，将 locked 设置为 true，获得锁
*/