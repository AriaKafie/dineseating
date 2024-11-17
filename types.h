
#ifndef TYPES_H
#define TYPES_H

#include <cstring>
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
    SharedData(int max_requests_) : max_requests(max_requests_)
    {
        memset(in_request_queue, 0, sizeof(unsigned int) * RequestTypeN);
        memset(produced,         0, sizeof(unsigned int) * RequestTypeN);
        memset(consumed[TX],     0, sizeof(unsigned int) * RequestTypeN);
        memset(consumed[Rev9],   0, sizeof(unsigned int) * RequestTypeN);

        pthread_cond_init(&cond_consumed,     NULL);
        pthread_cond_init(&cond_vip_consumed, NULL);
        pthread_cond_init(&cond_unconsumed,   NULL);
        
        sem_init(&main_blocker, VALUE_ZERO, VALUE_ZERO);
        
        /* sem_init(&consumed,     VALUE_ZERO, CAPACITY); */
        /* sem_init(&vip_consumed, VALUE_ZERO, VIP_CAPACITY); */
        /* sem_init(&unconsumed,   VALUE_ZERO, DEFAULT_UNCONSUMED_AVAILABLE); */        
        
        pthread_mutex_init(&lock, NULL);
    }
    
    pthread_mutex_t         lock;

    pthread_cond_t          cond_consumed;
    pthread_cond_t          cond_vip_consumed;
    pthread_cond_t          cond_unconsumed;
    sem_t                   main_blocker;
    
    /* sem_t                   consumed; */
    /* sem_t                   vip_consumed; */
    /* sem_t                   unconsumed; */
    
    int                     max_requests;
    unsigned int            consumed[ConsumerTypeN][RequestTypeN];
    unsigned int            produced[RequestTypeN];
    unsigned int            in_request_queue[RequestTypeN];
    std::queue<RequestType> requests;
};

#endif
