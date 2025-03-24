#include <iostream>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>

 class Foo{
    private:
        int x;
    public:
        Foo(){}
        Foo(int x_):x(x){}
 };

int main(){
   
    Foo	some_array[5];
    std::atomic<Foo*>p(some_array);
    Foo*x = p.fetch_add(2);		//	p加2，并返回原始值
    assert(x == some_array);
    //加载第二个元素的值
    assert(p.load() == &some_array[2]);
    //指针减1
    x = (p -= 1);		//	p减1，并返回原始值
    assert(x == &some_array[1]);
    assert(p.load() == &some_array[1]);
    return 0;
}