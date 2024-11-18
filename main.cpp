
#include <unistd.h>

#include "log.h"
#include "types.h"

#define SUCCESS 0
#define US_PER_MS 1000

void sleep(int ms) {
    usleep(ms * US_PER_MS);
}

template<RequestType T>
void *producer(void *ptr)
{
    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        // sleep to simulate production

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

int main(int argc, char **argv)
{
    SharedData sd(100);
    pthread_t general_producer, vip_producer, tx, r9;

    pthread_create(&general_producer, NULL, &producer<GeneralTable>, &sd);
    pthread_create(&vip_producer,     NULL, &producer<VIPRoom>,      &sd);
    pthread_create(&tx,               NULL, &consumer<TX>,           &sd);
    pthread_create(&r9,               NULL, &consumer<Rev9>,         &sd);
    
    sem_wait(&sd.main_blocker);

    unsigned int *consumed_[RequestTypeN] = { sd.consumed[TX], sd.consumed[Rev9] };
    
    output_production_history(sd.produced, consumed_);
}
