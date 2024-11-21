#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>


typedef void (*ThreadPoolFunction)(void *arg);

struct ThreadPoolWorkUnit {
    ThreadPoolFunction Function;
    void* Argument;
    struct ThreadPoolWorkUnit* NextWorkUnit;
};

struct ThreadPool
{
    struct ThreadPoolWorkUnit* FirstWorkUnit;
    struct ThreadPoolWorkUnit* LastWorkUnit;
    pthread_mutex_t work_mutex;
    pthread_cond_t work_cond;
    pthread_cond_t working_cond;
    size_t WorkingThreadCount;
    size_t TotalThreadCount;
    bool stop;
};

struct ThreadPoolWorkUnit *tpool_work_create(ThreadPoolFunction func, void* arg);
void tpool_work_destroy(struct ThreadPoolWorkUnit* workUnit);
struct ThreadPoolWorkUnit *tpool_work_get(struct ThreadPool* threadPool);
void *tpool_worker(void *arg);

struct ThreadPool* ThreadPool_Create(size_t threadCount);
void ThreadPool_Destroy(struct ThreadPool* threadPool);
bool ThreadPool_AddWork(struct ThreadPool* threadPool, ThreadPoolFunction func, void* arg);
void ThreadPool_Wait(struct ThreadPool* threadPool);


#endif // THREADPOOL_H