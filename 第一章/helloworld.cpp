#include <iostream>
#include <thread>

void print(){
    for(int i = 0; i < 10; i++){
        std::cout<<i<<std::endl;
    }
}

int main(){
    std::cout<<"sub thread is start"<<std::endl;
    std::thread t(print);
    t.join();
    std::cout<<"mian thread is over!"<<std::endl;
    return 0;
}