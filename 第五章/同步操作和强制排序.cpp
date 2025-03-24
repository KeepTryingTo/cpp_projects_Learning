#include <iostream>
#include <algorithm>
#include <vector>
#include <atomic>
#include <assert.h>
#include <thread>
#include <chrono>

std::vector<int>data;
std::atomic<bool>data_ready(false);

void reader_thread(){
    while(!data_ready.load()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout<<"The answer = "<<data[0]<<std::endl;
}

void writer_thread(){
    data.push_back(43);
    data_ready = true;
}

int main(){
    std::thread read(reader_thread);
    std::thread write(writer_thread);

    read.join();
    write.join();
    return 0;
}