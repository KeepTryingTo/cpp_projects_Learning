#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <chrono>
#include "timer.h"

namespace ktg {
    class TimerManager{
        public:
            TimerManager() = default;
            ~TimerManager() = default;
            template<typename F, typename... Args>
            void schedule(int milliseconds, F&& f, Args... args);

            template<typename F, typename... Args>
            void schedule(int milliseconds, int repeat, F&& f, Args... args);//指定最大触发次数

            void update();
        private:
            std::multimap<int64_t, Timer>m_timers;
    };

    template<typename F, typename... Args>
    void TimerManager::schedule(int milliseconds, F&& f, Args... args){
        schedule(milliseconds, -1, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //指定最大触发次数
    template<typename F, typename... Args>
    void TimerManager::schedule(int milliseconds, int repeat, F&& f, Args... args){
        Timer t(repeat);
        t.callback(milliseconds, std::forward<F>(f), std::forward<Args>(args)...);
        //记录当前时间点的定时器对象
        m_timers.insert(std::make_pair(t.m_time, t));
    }
}