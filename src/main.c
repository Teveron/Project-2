#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "threadpool.h"

static const size_t num_threads = 4;
static const size_t num_items   = 100;

void worker(void *arg)
{
    int *val = arg;
    int  old = *val;

    *val += 1000;
    printf("tid=%ld, old=%d, val=%d\n", pthread_self(), old, *val);

    if (*val%2)
        usleep(100000);
}

int main(int argc, char **argv)
{
    struct ThreadPool *tm;
    int     *vals;
    size_t   i;

    tm   = ThreadPool_Create(num_threads);
    vals = calloc(num_items, sizeof(*vals));

    for (i=0; i<num_items; i++) {
        vals[i] = i;
        ThreadPool_AddWork(tm, worker, vals+i);
    }

    ThreadPool_Wait(tm);

    for (i=0; i<num_items; i++) {
        printf("%d\n", vals[i]);
    }

    free(vals);
    ThreadPool_Destroy(tm);
    return 0;
}
