
#include "log.h"
#include "shared_data.h"

template<ConsumerType T>
void *consumer(void *ptr)
{    
    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        pthread_mutex_lock(&sd->lock);

        if (sd->consumed[TX][GeneralTable]   +
            sd->consumed[TX][VIPRoom]        +
            sd->consumed[Rev9][GeneralTable] +
            sd->consumed[Rev9][VIPRoom]      >= sd->max_requests)
            break;
        
        while (sd->requests.empty())
            pthread_cond_wait(&sd->cond_unconsumed, &sd->lock);
        
        RequestType rt = sd->requests.front();
        sd->requests.pop();
        sd->consumed[T][rt]++;
        sd->in_request_queue[rt]--;

        output_request_removed(T, rt, sd->consumed[T], sd->in_request_queue);

        pthread_mutex_unlock(&sd->lock);
        pthread_cond_signal(&sd->cond_consumed);
        if (rt == VIPRoom)
            pthread_cond_signal(&sd->cond_vip_consumed);

        // sleep to simulate consumption
    }
    
    sem_post(&sd->main_blocker);
    pthread_exit(SUCCESS);
}
