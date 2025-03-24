#include <iostream>
#include <chrono>
#include <condition_variable>

void fn(){
    for(int i = 0; i < 100000; i++){
        continue;
    }
}

//超时功能
std::condition_variable	cv;
bool done = true;
std::mutex	m;
bool wait_loop()
{
    //设定超时时间为500毫秒
    auto const timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    //上锁
    std::unique_lock<std::mutex>lk(m);
    //在时间未到达之前都要继续循环
    while(!done){
        if(cv.wait_until(lk,timeout)==std::cv_status::timeout){
            break;
        }       
    }
    return done;
}

int main(){
    //时延
    std::chrono::milliseconds ms(55802);
    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::cout<<"seconds = "<<s.count()<<std::endl;

    //时间点
    auto start = std::chrono::high_resolution_clock::now();
    fn();
    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> res(stop - start);
    std::cout<<res.count()<<std::endl;

    //验证超时功能
    if(wait_loop()){
        std::cout<<"time out is finished!"<<std::endl;
    }
    return 0;
}