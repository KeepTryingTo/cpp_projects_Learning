
#include <string>
#include <string.h>

#include "TaskQueue.h"

template<typename T>
TaskQueue<T>::TaskQueue(){
    pthread_mutex_init(&this -> m_mutex, NULL);
}
template<typename T>
TaskQueue<T>::~TaskQueue(){
    pthread_mutex_destroy(&this -> m_mutex);
}
template<typename T>
void TaskQueue<T>::addTask(Task<T> task){
    pthread_mutex_lock(&this -> m_mutex);
    this -> m_taskQ.push(task);
    pthread_mutex_unlock(&this -> m_mutex);
}
template<typename T>
void TaskQueue<T>::addTask(callback f, void * arg){
    pthread_mutex_lock(&this -> m_mutex);
    this -> m_taskQ.push(Task<T>(f, arg));
    pthread_mutex_unlock(&this -> m_mutex);
}
template<typename T>
Task<T> TaskQueue<T>::takeTask(){
    pthread_mutex_lock(&this -> m_mutex);
    Task<T> t;
    if(!this -> m_taskQ.empty()){
        t = this -> m_taskQ.front();
        this -> m_taskQ.pop();
    }
    pthread_mutex_unlock(&this -> m_mutex);
    return t;
}