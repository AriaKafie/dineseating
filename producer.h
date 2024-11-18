
// Aria Kafie 828109926
// Anosh Taraporevala 827939860

#include "log.h"
#include "shared_data.h"

template<RequestType T>
void *producer(void *ptr)
{
    // convert ptr to the appropriate type
    SharedData *sd = (SharedData*)ptr;

    // loop continuously
    for (;;)
    {
        // sleep to simulate production
        sleep(T == GeneralTable ? sd->general_time : sd->vip_time);

        // acquire the lock to access the shared data
        pthread_mutex_lock(&sd->lock);

        // sleep while the queue is at capacity
        if constexpr (T == GeneralTable)
            while (sd->in_request_queue[GeneralTable] + sd->in_request_queue[VIPRoom] >= CAPACITY)
                pthread_cond_wait(&sd->cond_consumed, &sd->lock);
        // if we're inserting VIP rooms, sleep while queue at capacity or VIP at capacity
        else
            while (!(sd->in_request_queue[GeneralTable] + sd->in_request_queue[VIPRoom] < CAPACITY && sd->in_request_queue[VIPRoom] < VIP_CAPACITY))
                pthread_cond_wait(sd->in_request_queue[VIPRoom] >= VIP_CAPACITY ? &sd->cond_vip_consumed : &sd->cond_consumed, &sd->lock);

        // if we've already inserted max_requests, stop
        if (sd->produced[GeneralTable] + sd->produced[VIPRoom] >= sd->max_requests)
        {
            pthread_mutex_unlock(&sd->lock);
            break;
        }

        // access the shared data, inserting a request
        sd->requests.push(T);
        sd->produced[T]++;
        sd->in_request_queue[T]++;

        // log the insertion
        output_request_added(T, sd->produced, sd->in_request_queue);

        // relinquish the lock and signal the consumers waiting on 'cond_unconsumed'
        pthread_mutex_unlock(&sd->lock);
        pthread_cond_signal(&sd->cond_unconsumed);
    }

    // exit with code 0
    pthread_exit(SUCCESS);
}
