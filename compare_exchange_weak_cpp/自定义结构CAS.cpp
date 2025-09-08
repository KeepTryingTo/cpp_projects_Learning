#include <iostream>
#include <atomic>

struct Point {
    int x;
    int y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

//int main() {
//    std::atomic<Point> atomic_point{ {0, 0} };
//
//    Point old_value = atomic_point.load();
//    Point new_value{ 1, 2 };
//
//    // 使用CAS更新Point
//    bool success = atomic_point.compare_exchange_strong(
//        old_value,
//        new_value
//    );
//
//    if (success) {
//        std::cout << "CAS succeeded: ("
//            << atomic_point.load().x << ", "
//            << atomic_point.load().y << ")" << std::endl;
//    }
//    else {
//        std::cout << "CAS failed - value was modified by another thread" << std::endl;
//    }
//
//    return 0;
//}