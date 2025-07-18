
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <pthread.h>

#include "threadPool.h"

template<typename T>
ThreadPool<T>::ThreadPool(int minNum, int maxNum){
    
    do {
        this -> taskQ = new TaskQueue<T>();
        if(taskQ == nullptr){
            std::cout<<"new task Queue is failed ..."<<std::endl;
            break;
        }
        this->threadIDs = new pthread_t[maxNum];
        if(this -> threadIDs == nullptr){
            std::cout<<"new threadIDs failed ..."<<std::endl;
            break;
        }
    
        memset(this -> threadIDs, 0, sizeof(pthread_t) * maxNum);
        this -> minNum = minNum;
        this -> maxNum = maxNum;
        this -> busyNum = 0;
        this -> liveNum = minNum;
        this -> exitNum = 0;
    
        if( pthread_mutex_init(&mutexPool, NULL) != 0 ||
            pthread_cond_init(&notEmpty, NULL) != 0){
                std::cout<<"mutex or condition init fail ..."<<std::endl;
                break;
            }
        
        this -> shutdown = false;
    
        //创建线程
        pthread_create(&this-> managerID, NULL, manager, this);
        for(int i = 0; i < minNum; i++){
            pthread_create(&this -> threadIDs[i], NULL, worker, this);
        }
        return;
    }while(0);

    //释放资源
    if(threadIDs)delete [] threadIDs;
    if(taskQ)delete taskQ;
    return ;
}
template<typename T>
void * ThreadPool<T>::manager(void * arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while(!pool -> shutdown){
        //每隔3秒检测一次
        sleep(3);

        //取出线程池中任务数量以及当前线程数量
        pthread_mutex_lock(&pool -> mutexPool);
        int queueSize = pool -> taskQ -> taskNumber();
        int liveNum = pool -> liveNum;
        int busyNum = pool -> busyNum;
        pthread_mutex_unlock(&pool -> mutexPool);

        // 添加线程
        if(queueSize > liveNum && liveNum < pool -> maxNum){
            pthread_mutex_lock(&pool -> mutexPool);
            int counter = 0;
            for(int i = 0; i < pool -> maxNum && counter < NUMBER
                    && pool -> liveNum < pool -> maxNum; i++){
                if(pool -> threadIDs[i] == 0){
                    pthread_create(&pool -> threadIDs[i], NULL, worker, pool);
                    counter += 1;
                    pool -> liveNum += 1;
                } 
            }
            pthread_mutex_unlock(&pool -> mutexPool);
        }

        //销毁线程
        if(busyNum * 2 < liveNum && liveNum > pool -> minNum){
            pthread_mutex_lock(&pool -> mutexPool);
            pool -> exitNum = NUMBER;
            pthread_mutex_unlock(&pool -> mutexPool);

            for(int i = 0; i < NUMBER; i++){
                pthread_cond_signal(&pool -> notEmpty);
            }
        }
    }
}

template<typename T>
void * ThreadPool<T>::worker(void * arg) {
    ThreadPool * pool = static_cast<ThreadPool*>(arg);

    while(1) {
        pthread_mutex_lock(&pool -> mutexPool);
        while(pool -> taskQ->taskNumber() == 0 && !pool -> shutdown){
            pthread_cond_wait(&pool -> notEmpty, &pool -> mutexPool);

            // 判断是不是要销毁线程
            if(pool -> exitNum > 0){
                pool -> exitNum -= 1;
                if(pool -> liveNum > pool -> minNum){
                    pool -> liveNum -= 1;
                    pthread_mutex_unlock(&pool -> mutexPool); 
                    pool -> threadExit();
                }
            }
        }

        //判断线程池是否被关闭
        if(pool -> shutdown){
            pthread_mutex_unlock(&pool -> mutexPool);
            pool -> threadExit();
        }

        //从任务队列中取出一个任务
        Task<T> task = pool -> taskQ -> takeTask();

        pool -> busyNum += 1;
        pthread_mutex_unlock(&pool -> mutexPool);

        std::cout<<"thread = "<<std::to_string(pthread_self())<<"start working..."<<std::endl;
        
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;

        std::cout<<"thread = "<<std::to_string(pthread_self())<<"end working..."<<std::endl;
        pthread_mutex_lock(&pool -> mutexPool);
        pool -> busyNum -= 1;
        pthread_mutex_unlock(&pool -> mutexPool);


    }
}

template<typename T>
void ThreadPool<T>::addTask(Task<T> task) {
    if(shutdown)return;
    // 添加任务
    taskQ -> addTask(task);
    pthread_cond_signal(&notEmpty);
}

template<typename T>
int ThreadPool<T>::getThreadPoolBusyNum(){
    pthread_mutex_lock(&mutexPool);
    int busyNum = this -> busyNum;
    pthread_mutex_unlock(&mutexPool);
    return busyNum;
}

template<typename T>
int ThreadPool<T>::getThreadPoolAliveNum() {
    pthread_mutex_lock(&mutexPool);
    int aliveNum = this -> liveNum;
    pthread_mutex_unlock(&mutexPool);
    return aliveNum;
}

template<typename T>
void ThreadPool<T>::threadExit() {
    pthread_t pid = pthread_self();
    for(int i = 0; i < maxNum; i++){
        if(threadIDs[i] == pid) {
            threadIDs[i] = 0;
            std::cout<<"thread destroy called = "<<std::to_string(pid)<<" exiting..."<<std::endl;
            break;
        }
    }
    pthread_exit(NULL);
}

template<typename T>
ThreadPool<T>::~ThreadPool() {
    //关闭线程池
    shutdown = true;
    //阻塞回收管理者线程
    pthread_join(managerID, NULL);
    //唤醒阻塞的消费者线程
    for(int i = 0; i < liveNum; i++){
        pthread_cond_signal(&notEmpty);
    }
    //释放堆内存
    if(taskQ)delete taskQ;
    if(threadIDs)delete [] threadIDs; 
    pthread_mutex_destroy(&mutexPool);
    pthread_cond_destroy(&notEmpty);
}