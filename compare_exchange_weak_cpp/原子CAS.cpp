#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> counter(0);

void increment_counter() {
    int expected;
    do {
        expected = counter.load(); // ��ȡ��ǰֵ
    } while (!counter.compare_exchange_weak(expected, expected + 1));
    // ���counter == expected��������Ϊexpected + 1������expected ����Ϊcounterֵ
}

void test() {
const int num_threads = 10;
    std::thread threads[num_threads];

    // ����10���߳�
    for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread(increment_counter);
    }
    // ���̵߳ȴ����߳�ִ�����
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