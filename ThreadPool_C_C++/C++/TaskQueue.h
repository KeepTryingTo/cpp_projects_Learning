#pragma once

#include <queue>
#include <pthread.h>
#include <string>
#include <string.h>

using callback = void (*)(void * arg);

//任务结构体
template <typename T>
struct Task {
    callback function;
    T * arg;

    Task(){
        this -> function = nullptr;
        this -> arg = nullptr;
    }
    Task(callback f, void * arg){
        this -> function = f;
        this -> arg = (T*)arg;
    }
};

template<typename T>
class TaskQueue {
public:
    TaskQueue();

    //添加任务  
    void addTask(Task<T> task);
    void addTask(callback f, void * arg);
    // 取出任务
    Task<T> takeTask();
    // 获取当前任务的个数
    inline size_t taskNumber(){
        return m_taskQ.size();
    }

    ~TaskQueue();
private:
    std::queue<Task<T>>m_taskQ;
    pthread_mutex_t m_mutex;
};