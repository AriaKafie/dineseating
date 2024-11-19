
// Aria Kafie 828109926
// Anosh Taraporevala 827939860

#ifndef SHARED_DATA
#define SHARED_DATA

#include <cstring>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <unistd.h>

#include "seating.h"

// define some constants
#define SUCCESS      0
#define US_PER_MS    1000
#define CAPACITY     18
#define VIP_CAPACITY 5
#define VALUE_ZERO   0

/**
 * @brief     Sleep for some number of milliseconds
 *
 * @param ms  The number of milliseconds to sleep for
*/
inline void sleep(int ms) { usleep(ms * US_PER_MS); }

// structure to hold the shared data
struct SharedData
{
    /**
     * @brief    Construct a SharedData object and initialize its variables
     *
     * @param s  The total number of requests to process
     * @param x  The number of milliseconds TX should take to consume a request
     * @param r  The number of milliseconds Rev9 should take to consume a request
     * @param g  The number of milliseconds it should take to produce a general table request
     * @param v  The number of milliseconds it should take to produce a VIP room request
    */
    SharedData(int s, int x, int r, int g, int v) : max_requests(s),
                                                    tx_time     (x),
                                                    r9_time     (r),
                                                    general_time(g),
                                                    vip_time    (v)
    {
        // zero-initialize various data
        memset(in_request_queue, 0, sizeof(unsigned int) * RequestTypeN);
        memset(produced,         0, sizeof(unsigned int) * RequestTypeN);
        memset(consumed[TX],     0, sizeof(unsigned int) * RequestTypeN);
        memset(consumed[Rev9],   0, sizeof(unsigned int) * RequestTypeN);

        // initialize the conditional variables
        pthread_cond_init(&cond_consumed,     NULL);
        pthread_cond_init(&cond_vip_consumed, NULL);
        pthread_cond_init(&cond_unconsumed,   NULL);

        // initialize the semaphore
        sem_init(&main_blocker, VALUE_ZERO, VALUE_ZERO);

        // initialize the lock
        pthread_mutex_init(&lock, NULL);
    }

    // command line arguments
    int max_requests;
    int tx_time;
    int r9_time;
    int general_time;
    int vip_time;

    // mutex for accessing the shared data
    pthread_mutex_t         lock;

    // conditional variables for waiting and signaling
    pthread_cond_t          cond_consumed;
    pthread_cond_t          cond_vip_consumed;
    pthread_cond_t          cond_unconsumed;

    // semaphore to block the main thread
    sem_t                   main_blocker;

    // arrays to hold various consumption/production counters
    unsigned int            consumed[ConsumerTypeN][RequestTypeN];
    unsigned int            produced[RequestTypeN];
    unsigned int            in_request_queue[RequestTypeN];

    // queue to hold the requests
    std::queue<RequestType> requests;
};

#endif
