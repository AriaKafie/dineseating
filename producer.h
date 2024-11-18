
#include "log.h"
#include "shared_data.h"

template<RequestType T>
void *producer(void *ptr)
{
    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        // sleep to simulate production
        sleep(T == GeneralTable ? sd->general_time : sd->vip_time);

        pthread_mutex_lock(&sd->lock);

        while (sd->in_request_queue[GeneralTable] + sd->in_request_queue[VIPRoom] >= CAPACITY)
            pthread_cond_wait(&sd->cond_consumed, &sd->lock);

        if (T == VIPRoom)
            while (sd->in_request_queue[VIPRoom] >= VIP_CAPACITY)
                pthread_cond_wait(&sd->cond_vip_consumed, &sd->lock);

        if (sd->produced[GeneralTable] + sd->produced[VIPRoom] >= sd->max_requests)
        {
            pthread_mutex_unlock(&sd->lock);
            break;
        }
        
        sd->requests.push(T);
        sd->produced[T]++;
        sd->in_request_queue[T]++;
        
        output_request_added(T, sd->produced, sd->in_request_queue);

        pthread_mutex_unlock(&sd->lock);
        pthread_cond_signal(&sd->cond_unconsumed);
    }
    
    pthread_exit(SUCCESS);
}
