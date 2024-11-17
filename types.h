
#ifndef TYPES_H
#define TYPES_H

#include <pthread.h>
#include <queue>
#include <semaphore.h>

#include "seating.h"

#define CAPACITY 18
#define VIP_CAPACITY 5
#define DEFAULT_UNCONSUMED_AVAILABLE 0
#define VALUE_ZERO 0

struct SharedData
{
    SharedData(int max_requests_) : max_requests            (max_requests_),
                                    general_produced        (0),
                                    vip_produced            (0),
                                    total_consumed          (0),
                                    general_in_request_queue(0),
                                    vip_in_request_queue    (0)
    {
        sem_init(&consumed,     VALUE_ZERO, CAPACITY);
        sem_init(&vip_consumed, VALUE_ZERO, VIP_CAPACITY);
        sem_init(&unconsumed,   VALUE_ZERO, DEFAULT_UNCONSUMED_AVAILABLE);
        sem_init(&main_blocker, VALUE_ZERO, VALUE_ZERO);
        
        pthread_mutex_init(&lock, NULL);
    }
    
    pthread_mutex_t         lock;
    sem_t                   consumed;
    sem_t                   vip_consumed;
    sem_t                   unconsumed;
    sem_t                   main_blocker;
    int                     max_requests;
    int                     general_produced;
    int                     vip_produced;
    int                     total_consumed;
    int                     general_in_request_queue;
    int                     vip_in_request_queue;
    std::queue<RequestType> requests;
};

#endif
