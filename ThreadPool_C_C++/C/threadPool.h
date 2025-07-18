#ifndef __THREADPOOL_H
#define __THREADPOOL_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


typedef struct ThreadPool ThreadPool;

ThreadPool * threadPoolCreate(int minNum, int maxNum, int queueSize);
// 工作线程函数
void * worker(void * arg);
// 管理线程函数
void * manager(void * arg);

//销毁线程
void threadDestroy(ThreadPool * pool);
int threadPoolDestroy(ThreadPool * pool);

//向线程池添加任务
void threadPoolAdd(ThreadPool * pool, void(*func)(void *), void *arg);

//线程池中工作的线程数量以及活着的线程个数
int threadPoolBusyNum(ThreadPool * pool);
int threadPoolAliveNum(ThreadPool * pool);



#endif 
