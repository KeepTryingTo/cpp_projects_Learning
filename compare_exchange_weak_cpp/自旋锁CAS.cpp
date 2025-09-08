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
            // ����expectedֵ������ѭ�����Ӷ��ﵽ��������Ŀ��
            expected = false; 
            // ��ѡ: ����ȴ����Լ���CPUռ��
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
1. ��ʼ״̬
    locked ��ʼֵΪ false��δ������
    expected ��ʼֵΪ false

2. ��һ���߳̽���lock()
    compare_exchange_weak ��飺locked == expected��false == false��
    �������������ǽ� locked ����Ϊ true���������� true
    whileѭ������ !true Ϊ false���˳�ѭ�����̻߳����

3. �ڶ����߳̽���lock()
    ��ʱ locked �Ѿ��� true
    compare_exchange_weak ��飺locked == expected��true == false��
    �������������������� false

    whileѭ������ !false Ϊ true������ѭ��
    ���� expected = false����������

4. ��һ���̵߳���unlock()
    locked.store(false) �����ͷ�

5. �ڶ����߳��ٴγ���
    compare_exchange_weak ��飺locked == expected��false == false��
    ������������ locked ����Ϊ true�������
*/