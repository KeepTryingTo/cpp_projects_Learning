
#include <iostream>
#include <algorithm>
#include <string>
#include "timer.h"

using namespace ktg;

void fun(){
    std::cout<<"无参函数"<<std::endl;
}

void fun1(int i){
    std::cout<<"有参函数 "<<i<<std::endl;
}

int main(){
    Timer t1;
    t1.start(2000, fun);

    Timer t2;
    int i = 0;
    t2.start(2000, fun1, i++);

    std::getchar();
    return 0;
}

