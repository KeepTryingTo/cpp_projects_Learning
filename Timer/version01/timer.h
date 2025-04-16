#pragma once

#include <thread>
#include <iostream>
#include <functional>
#include <atomic>


namespace ktg {
    class Timer{
        public:
            Timer();
            Timer(int repeat);
            ~Timer();
            template<typename F,typename... Args>
            void start(int milliseconds, F&& func, Args&&... args);//启动定时器
            void stop();//关闭定时器
        private:
            std::thread m_thread;
            std::atomic<bool>m_active;
            std::function<void()>m_func;
            int m_period;//定时器执行的周期多长，单位毫秒
            int m_repeat;//定时器重复触发的次数
    };
    //启动定时器
    template<typename F,typename... Args>
    void Timer::start(int milliseconds, F&& func, Args&&... args){
        if(m_active.load()){
            return;
        }
        m_period = milliseconds;
        m_func = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
        m_active.store(true);
        
        m_thread = std::thread([&](){
            if(m_repeat < 0){//无限循环定时器
                while(m_active.load()){
                    std::this_thread::sleep_for(std::chrono::milliseconds(m_period));
                    if(!m_active.load()){
                        return;
                    }
                    m_func();
                }
            }else{
                while(m_repeat > 0){
                    if(!m_active.load()){
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(m_period));
                    m_func();
                    if(!m_active.load()){
                        return;
                    }
                    m_func();
                    m_repeat--;
                }
            }
        });
        m_thread.detach();
    }
}