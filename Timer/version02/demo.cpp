
#include <iostream>
#include <algorithm>
#include <string>
#include "timer.h"
#include "timerManager.h"
using namespace ktg;

void fun(){
    std::cout<<"无参函数"<<std::endl;
}

void fun1(int i){
    std::cout<<"有参函数 "<<i<<std::endl;
}

int main(){
    
    TimerManager tmg;
    tmg.schedule(1000, fun);
    tmg.schedule(1500, fun1, 1);

    while(true){
        tmg.update();
    }
    return 0;
}

