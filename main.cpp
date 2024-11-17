
#include <iostream>
#include <unistd.h>

#include "log.h"
#include "types.h"

#define SUCCESS 0
#define US_PER_MS 1000

void sleep(int ms) {
    usleep(ms * US_PER_MS);
}

void *produce_general(void *ptr)
{
    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        // sleep to simulate production

        sem_wait(&sd->consumed);
        pthread_mutex_lock(&sd->lock);

        if (sd->produced[GeneralTable] + sd->produced[VIPRoom] >= sd->max_requests)
        {
            pthread_mutex_unlock(&sd->lock);
            sem_post(&sd->unconsumed);
            break;
        }
        
        sd->requests.push(GeneralTable);
        sd->produced[GeneralTable]++;
        sd->in_request_queue[GeneralTable]++;
        
        output_request_added(GeneralTable, sd->produced, sd->in_request_queue);

        pthread_mutex_unlock(&sd->lock);
        sem_post(&sd->unconsumed);
    }
    
    pthread_exit(SUCCESS);
}

void *produce_vip(void *ptr)
{
    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        // sleep to simulate production

        sem_wait(&sd->consumed);
        sem_wait(&sd->vip_consumed);
        pthread_mutex_lock(&sd->lock);

        if (sd->produced[GeneralTable] + sd->produced[VIPRoom] >= sd->max_requests)
        {
            pthread_mutex_unlock(&sd->lock);
            sem_post(&sd->unconsumed);
            break;
        }
        
        sd->requests.push(VIPRoom);
        sd->produced[VIPRoom]++;
        sd->in_request_queue[VIPRoom]++;
        
        output_request_added(VIPRoom, sd->produced, sd->in_request_queue);
        
        pthread_mutex_unlock(&sd->lock);
        sem_post(&sd->unconsumed);
    }
    
    pthread_exit(SUCCESS);
}

void *consume(void *ptr)
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

        output_request_removed(TX, rt, consumed, sd->in_request_queue);

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

    pthread_create(&general_producer, NULL, &produce_general, &s);
    pthread_create(&vip_producer, NULL, &produce_vip, &s);
    pthread_create(&tx, NULL, &consume, &s);
    
    sem_wait(&s.main_blocker);

    std::cout << "Done" << std::endl;
}
