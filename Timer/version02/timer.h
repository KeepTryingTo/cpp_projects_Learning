#pragma once

#include <thread>
#include <iostream>
#include <functional>
#include <chrono>
#include <atomic>


namespace ktg {
    class Timer{
        friend class TimerManager;
        public:
            Timer();
            Timer(int repeat);
            ~Timer();
            template<typename F,typename... Args>
            void callback(int milliseconds, F&& func, Args&&... args);//启动定时器
            void on_timer();
        private:
            static int64_t now();//获取系统时间
        private:
            int64_t m_time;//定时器触发的时间点，毫秒
            std::function<void()>m_func;
            int m_period;//定时器执行的周期多长，单位毫秒
            int m_repeat;//定时器重复触发的次数
    };
    //启动定时器
    template<typename F,typename... Args>
    void Timer::callback(int milliseconds, F&& func, Args&&... args){
        m_period = milliseconds;
        m_func = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
    }
}