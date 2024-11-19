
// Aria Kafie 828109926
// Anosh Taraporevala 827939860

#ifndef CONSUMER_H
#define CONSUMER_H

#include "log.h"
#include "shared_data.h"

/**
 * @brief      Continually consume seating requests from the shared queue
 *
 * @param T    The ConsumerType we are simulating (TX or Rev9)
 * @param ptr  A pointer to the shared data structure holding seating information
*/
template<ConsumerType T>
void *consumer(void *ptr)
{
    // convert ptr to the appropriate type
    SharedData *sd = (SharedData*)ptr;

    // loop continuously
    for (;;)
    {
        // acquire the lock
        pthread_mutex_lock(&sd->lock);

        // stop consuming once total requests have been consumed
        if (sd->consumed[TX][GeneralTable]   +
            sd->consumed[TX][VIPRoom]        +
            sd->consumed[Rev9][GeneralTable] +
            sd->consumed[Rev9][VIPRoom]      >= sd->max_requests)
            break;

        // sleep while there is nothing to consume
        while (sd->requests.empty())
            pthread_cond_wait(&sd->cond_unconsumed, &sd->lock);

        // access the shared data
        RequestType rt = sd->requests.front();
        
        // remove the front of the queue
        sd->requests.pop();

        // update queue and consumption counters
        sd->consumed[T][rt]++;
        sd->in_request_queue[rt]--;

        // log the request removal
        output_request_removed(T, rt, sd->consumed[T], sd->in_request_queue);

        // relinquish the lock and signal producers waiting on cond_consumed
        pthread_mutex_unlock(&sd->lock);
        pthread_cond_signal(&sd->cond_consumed);

        // if we removed a vip request, signal producers waiting on VIP
        if (rt == VIPRoom)
            pthread_cond_signal(&sd->cond_vip_consumed);

        // sleep to simulate consumption
        sleep(T == TX ? sd->tx_time : sd->r9_time);
    }

    // signal the main thread that we are done
    sem_post(&sd->main_blocker);

    // exit with code 0
    pthread_exit(SUCCESS);
}

#endif
