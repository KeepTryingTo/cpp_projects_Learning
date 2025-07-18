#pragma once



#include "TaskQueue.h"

template<typename T>
class ThreadPool {
public:
    ThreadPool(int minNum, int maxNum);

    //销毁线程
    ~ThreadPool();

    //向线程池添加任务
    void addTask(Task<T> task);

    //线程池中工作的线程数量以及活着的线程个数
    int getThreadPoolBusyNum();
    int getThreadPoolAliveNum();

private:
    // 工作线程函数
    static void * worker(void * arg);
    // 管理线程函数
    static void * manager(void * arg);
    // 单线程退出
    void threadExit();


    // 任务队列
    TaskQueue<T> * taskQ;

    pthread_t managerID; // 管理者线程ID
    pthread_t * threadIDs; // 工作的线程ID
    int minNum; //最小线程数
    int maxNum; // 最大线程数
    int busyNum; // 忙的线程个数
    int liveNum; // 存活的线程个数
    int exitNum; // 要销毁的线程个数

    pthread_mutex_t mutexPool; // 锁整个的线程池
    pthread_cond_t notEmpty; // 任务队列是不是为空了

    bool shutdown; //是不是要销毁线程池，销毁为1，否则为0

    static const int NUMBER = 2;
};
