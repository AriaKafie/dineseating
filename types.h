
#ifndef TYPES_H
#define TYPES_H

#include <pthread.h>
#include <queue>
#include <semaphore.h>

#define CAPACITY 18
#define DEFAULT_UNCONSUMED_AVAILABLE 0
#define VALUE_ZERO 0

enum RequestType { GENERAL, VIP };

struct SharedData
{
    SharedData(int max_requests) : lock        (PTHREAD_MUTEX_INITIALIZER),
                                   max_requests(max_requests)
    {
        sem_init(&consumed, VALUE_ZERO, CAPACITY);
        sem_init(&unconsumed, VALUE_ZERO, DEFAULT_UNCONSUMED_AVAILABLE);
        sem_init(&main_blocker, VALUE_ZERO, VALUE_ZERO);
    }
    
    pthread_mutex_t         lock;
    sem_t                   consumed;
    sem_t                   unconsumed;
    sem_t                   main_blocker;
    int                     max_requests;
    std::queue<RequestType> requests;
};

#endif
