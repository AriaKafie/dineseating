
#ifndef SHARED_DATA
#define SHARED_DATA

#include <cstring>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <unistd.h>

#include "seating.h"

#define SUCCESS      0
#define US_PER_MS    1000
#define CAPACITY     18
#define VIP_CAPACITY 5
#define VALUE_ZERO   0

inline void sleep(int ms) { usleep(ms * US_PER_MS); }

struct SharedData
{
    SharedData(int s, int x, int r, int g, int v) : max_requests(s),
                                                    tx_time     (x),
                                                    r9_time     (r),
                                                    general_time(g),
                                                    vip_time    (v)
    {
        memset(in_request_queue, 0, sizeof(unsigned int) * RequestTypeN);
        memset(produced,         0, sizeof(unsigned int) * RequestTypeN);
        memset(consumed[TX],     0, sizeof(unsigned int) * RequestTypeN);
        memset(consumed[Rev9],   0, sizeof(unsigned int) * RequestTypeN);

        pthread_cond_init(&cond_consumed,     NULL);
        pthread_cond_init(&cond_vip_consumed, NULL);
        pthread_cond_init(&cond_unconsumed,   NULL);
        
        sem_init(&main_blocker, VALUE_ZERO, VALUE_ZERO);

        pthread_mutex_init(&lock, NULL);
    }

    int max_requests; // command line arguments
    int tx_time;
    int r9_time;
    int general_time;
    int vip_time;
    
    pthread_mutex_t         lock; // mutex for accessing shared data
    pthread_cond_t          cond_consumed;
    pthread_cond_t          cond_vip_consumed;
    pthread_cond_t          cond_unconsumed;
    sem_t                   main_blocker;    
    unsigned int            consumed[ConsumerTypeN][RequestTypeN];
    unsigned int            produced[RequestTypeN];
    unsigned int            in_request_queue[RequestTypeN];
    std::queue<RequestType> requests;
};

#endif
