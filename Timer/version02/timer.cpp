
#include <algorithm>
#include "timer.h"

using namespace ktg;

Timer::Timer():m_period(0),m_repeat(-1){
    m_time = now();
}
Timer::Timer(int repeat):m_period(0),m_repeat(repeat){
    m_time = now();
}
Timer::~Timer(){};
int64_t Timer::now(){
    //获取当前系统时间
    auto now = std::chrono::system_clock::now();
    //将系统时间转换为毫秒
    auto now_ms = std::chrono::time_point_cast<std::chrono::microseconds>(now);

    return now_ms.time_since_epoch().count();
}

void Timer::on_timer(){
    //当前回调函数还没有准备好
    if(!m_func || m_repeat == 0){
        return ;
    }
    m_func();
    m_time += m_period;
    if(m_repeat > 0){
        m_repeat--;
    }
}


