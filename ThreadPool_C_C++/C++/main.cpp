#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include "threadPool.cpp"
#include "TaskQueue.cpp"

void taskFunc(void *arg) {
    int num = *(int*)arg;
    std::cout<<"thread = " <<pthread_self()<<"is working, num = "<<num<<std::endl;
    sleep(3);
}

int main(){
    ThreadPool<int> pool(3, 10);
    for(int i = 0; i < 100; i++){
        int * num = new int(i + 100);
        pool.addTask(Task<int>(taskFunc, num));
    }
    sleep(30);
    return 0;
}