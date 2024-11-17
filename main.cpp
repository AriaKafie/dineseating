
#include <iostream>
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

        sem_wait(&sd->consumed);
        if (T == VIPRoom)
            sem_wait(&sd->vip_consumed);
        pthread_mutex_lock(&sd->lock);

        if (sd->produced[GeneralTable] + sd->produced[VIPRoom] >= sd->max_requests)
        {
            pthread_mutex_unlock(&sd->lock);
            sem_post(&sd->unconsumed);
            break;
        }
        
        sd->requests.push(T);
        sd->produced[T]++;
        sd->in_request_queue[T]++;
        
        output_request_added(T, sd->produced, sd->in_request_queue);

        pthread_mutex_unlock(&sd->lock);
        sem_post(&sd->unconsumed);
    }
    
    pthread_exit(SUCCESS);
}

template<ConsumerType T>
void *consumer(void *ptr)
{
    unsigned int consumed[RequestTypeN] = {};
    
    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        sem_wait(&sd->unconsumed);
        pthread_mutex_lock(&sd->lock);

        if (sd->total_consumed >= sd->max_requests)
            break;
        
        RequestType rt = sd->requests.front();
        sd->requests.pop();
        sd->total_consumed++;

        consumed[rt]++;
        sd->in_request_queue[rt]--;

        output_request_removed(T, rt, consumed, sd->in_request_queue);

        pthread_mutex_unlock(&sd->lock);
        sem_post(&sd->consumed);
        if (rt == VIPRoom)
            sem_post(&sd->vip_consumed);

        //sleep(100);
    }
    
    sem_post(&sd->main_blocker);
    pthread_exit(SUCCESS);
}

int main(int argc, char **argv)
{
    SharedData s(100);
    pthread_t general_producer, vip_producer, tx, r9;

    pthread_create(&general_producer, NULL, &producer<GeneralTable>, &s);
    pthread_create(&vip_producer,     NULL, &producer<VIPRoom>,      &s);
    pthread_create(&tx,               NULL, &consumer<TX>,           &s);
    pthread_create(&r9,               NULL, &consumer<Rev9>,         &s);
    
    sem_wait(&s.main_blocker);
}
