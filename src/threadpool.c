#include "threadpool.h"

struct ThreadPoolWorkUnit *tpool_work_create(ThreadPoolFunction func, void* arg)
{
    if (func == NULL)
        return NULL;

    struct ThreadPoolWorkUnit* workUnit = malloc(sizeof(*workUnit));
    workUnit->Function = func;
    workUnit->Argument  = arg;
    workUnit->NextWorkUnit = NULL;

    return workUnit;
}

void tpool_work_destroy(struct ThreadPoolWorkUnit* workUnit)
{
    if (workUnit == NULL)
        return;

    free(workUnit);
}

struct ThreadPoolWorkUnit *tpool_work_get(struct ThreadPool* threadPool)
{
    if (threadPool == NULL)
        return NULL;
    
    struct ThreadPoolWorkUnit* workUnit = threadPool->FirstWorkUnit;
    if (workUnit == NULL)
        return NULL;

    if (workUnit->NextWorkUnit == NULL)
    {
        threadPool->FirstWorkUnit = NULL;
        threadPool->LastWorkUnit  = NULL;
    }
    else
        threadPool->FirstWorkUnit = workUnit->NextWorkUnit;

    return workUnit;
}

void *tpool_worker(void *arg)
{
    struct ThreadPool* threadPool = arg;
    struct ThreadPoolWorkUnit* workUnit;

    while (1)
    {
        pthread_mutex_lock(&(threadPool->work_mutex));

        while (threadPool->FirstWorkUnit == NULL && !threadPool->stop)
            pthread_cond_wait(&(threadPool->work_cond), &(threadPool->work_mutex));

        if (threadPool->stop)
            break;

        workUnit = tpool_work_get(threadPool);
        threadPool->WorkingThreadCount++;
        pthread_mutex_unlock(&(threadPool->work_mutex));

        if (workUnit != NULL)
        {
            workUnit->Function(workUnit->Argument);
            tpool_work_destroy(workUnit);
        }

        pthread_mutex_lock(&(threadPool->work_mutex));
        threadPool->WorkingThreadCount--;
        if (!threadPool->stop && threadPool->WorkingThreadCount == 0 && threadPool->FirstWorkUnit == NULL)
            pthread_cond_signal(&(threadPool->working_cond));
        pthread_mutex_unlock(&(threadPool->work_mutex));
    }

    threadPool->TotalThreadCount--;
    pthread_cond_signal(&(threadPool->working_cond));
    pthread_mutex_unlock(&(threadPool->work_mutex));
    return NULL;
}


struct ThreadPool* ThreadPool_Create(size_t threadCount)
{
    struct ThreadPool* threadPool;
    pthread_t thread;

    if (threadCount == 0)
        threadCount = 2;

    threadPool = calloc(1, sizeof(*threadPool));
    threadPool->TotalThreadCount = threadCount;

    pthread_mutex_init(&(threadPool->work_mutex), NULL);
    pthread_cond_init(&(threadPool->work_cond), NULL);
    pthread_cond_init(&(threadPool->working_cond), NULL);

    threadPool->FirstWorkUnit = NULL;
    threadPool->LastWorkUnit  = NULL;

    size_t i;
    for (i = 0; i < threadCount; i++)
    {
        pthread_create(&thread, NULL, tpool_worker, threadPool);
        pthread_detach(thread);
    }

    return threadPool;
}

void ThreadPool_Destroy(struct ThreadPool* threadPool)
{
    if (threadPool == NULL)
        return;

    struct ThreadPoolWorkUnit* currWorkUnit;
    struct ThreadPoolWorkUnit* nextWorkUnit;

    pthread_mutex_lock(&(threadPool->work_mutex));
    currWorkUnit = threadPool->FirstWorkUnit;
    while (currWorkUnit != NULL)
    {
        nextWorkUnit = currWorkUnit->NextWorkUnit;
        tpool_work_destroy(currWorkUnit);
        currWorkUnit = nextWorkUnit;
    }

    threadPool->FirstWorkUnit = NULL;
    threadPool->stop = true;
    pthread_cond_broadcast(&(threadPool->work_cond));
    pthread_mutex_unlock(&(threadPool->work_mutex));

    ThreadPool_Wait(threadPool);

    pthread_mutex_destroy(&(threadPool->work_mutex));
    pthread_cond_destroy(&(threadPool->work_cond));
    pthread_cond_destroy(&(threadPool->working_cond));

    free(threadPool);
}

bool ThreadPool_AddWork(struct ThreadPool* threadPool, ThreadPoolFunction func, void* arg)
{
    if (threadPool == NULL)
        return false;

    struct ThreadPoolWorkUnit* workUnit = tpool_work_create(func, arg);
    if (workUnit == NULL)
        return false;

    pthread_mutex_lock(&(threadPool->work_mutex));
    if (threadPool->FirstWorkUnit == NULL)
    {
        threadPool->FirstWorkUnit = workUnit;
        threadPool->LastWorkUnit  = threadPool->FirstWorkUnit;
    }
    else
    {
        threadPool->LastWorkUnit->NextWorkUnit = workUnit;
        threadPool->LastWorkUnit = workUnit;
    }

    pthread_cond_broadcast(&(threadPool->work_cond));
    pthread_mutex_unlock(&(threadPool->work_mutex));

    return true;
}

void ThreadPool_Wait(struct ThreadPool* threadPool)
{
    if (threadPool == NULL)
        return;

    pthread_mutex_lock(&(threadPool->work_mutex));
    while (1)
    {
        if (threadPool->FirstWorkUnit != NULL
            || (!threadPool->stop && threadPool->WorkingThreadCount != 0)
            || (threadPool->stop && threadPool->TotalThreadCount != 0))
            pthread_cond_wait(&(threadPool->working_cond), &(threadPool->work_mutex));
        else
            break;
    }
    pthread_mutex_unlock(&(threadPool->work_mutex));
}