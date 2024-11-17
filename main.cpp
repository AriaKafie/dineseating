
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
    unsigned int produced[RequestTypeN];
    unsigned int in_request_queue[RequestTypeN];

    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        // sleep to simulate production

        sem_wait(&sd->consumed);
        pthread_mutex_lock(&sd->lock);

        if (sd->general_produced + sd->vip_produced >= sd->max_requests)
        {
            pthread_mutex_unlock(&sd->lock);
            sem_post(&sd->unconsumed);
            break;
        }
        
        sd->requests.push(GeneralTable);
        sd->general_produced++;
        sd->general_in_request_queue++;

        produced[GeneralTable] = sd->general_produced;
        produced[VIPRoom] = sd->vip_produced;

        in_request_queue[GeneralTable] = sd->general_in_request_queue;
        in_request_queue[VIPRoom] = sd->vip_in_request_queue;
        
        output_request_added(GeneralTable, produced, in_request_queue);

        pthread_mutex_unlock(&sd->lock);
        sem_post(&sd->unconsumed);
    }
    
    pthread_exit(SUCCESS);
}

void *produce_vip(void *ptr)
{
    unsigned int produced[RequestTypeN];
    unsigned int in_request_queue[RequestTypeN];
    
    SharedData *sd = (SharedData*)ptr;

    for (;;)
    {
        // sleep to simulate production

        sem_wait(&sd->consumed);
        sem_wait(&sd->vip_consumed);
        pthread_mutex_lock(&sd->lock);

        if (sd->general_produced + sd->vip_produced >= sd->max_requests)
        {
            pthread_mutex_unlock(&sd->lock);
            sem_post(&sd->unconsumed);
            break;
        }
        
        sd->requests.push(VIPRoom);
        sd->vip_produced++;
        sd->vip_in_request_queue++;

        produced[GeneralTable] = sd->general_produced;
        produced[VIPRoom] = sd->vip_produced;

        in_request_queue[GeneralTable] = sd->general_in_request_queue;
        in_request_queue[VIPRoom] = sd->vip_in_request_queue;
        
        output_request_added(VIPRoom, produced, in_request_queue);
        
        pthread_mutex_unlock(&sd->lock);
        sem_post(&sd->unconsumed);
    }
    
    pthread_exit(SUCCESS);
}

void *consume(void *ptr)
{
    RequestType consumed[RequestTypeN];
    
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

        if (rt == GeneralTable)
            sd->general_in_request_queue--;
        else
            sd->vip_in_request_queue--;

        std::cout << "consumed " << (rt == GeneralTable ? "general" : "vip") << std::endl;

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
