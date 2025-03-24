#include<atomic>
#include<thread>
#include<assert.h>
#include <iostream>

std::atomic<bool>	x,y;
std::atomic<int>	z;
void write_x_then_y(){
    x.store(true,std::memory_order_relaxed);		//	1
    y.store(true,std::memory_order_relaxed);		//	2
 }
 void read_y_then_x()
 {
    while(!y.load(std::memory_order_relaxed));		//	3
    if(x.load(std::memory_order_relaxed))		//	4
            ++z;
 }
 int main()
 {
    x=false;
    y=false;
    z=0;
    std::thread	a(write_x_then_y);
    std::thread	b(read_y_then_x);
    a.join();
    b.join();
    assert(z.load()!=0);		//	5
    std::cout<<"z = "<<z<<std::endl;
    return 0;
 }