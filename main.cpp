
#include <iostream>

#include <thread>
#include <chrono>

#include "types.h"

#define SUCCESS 0

void *produce_general(void *ptr)
{
    SharedData *sd = (SharedData*)ptr;

    pthread_exit(SUCCESS);
}

void *produce_vip(void *ptr)
{
    SharedData *sd = (SharedData*)ptr;

    pthread_exit(SUCCESS);
}

void *consumer(void *ptr)
{
    
}

int main(int argc, char **argv)
{
    SharedData s(100);
    pthread_t general_producer, vip_producer, tx,;

    pthread_create(&g, NULL, &produce_general, &s);
    pthread_create(&v, NULL, &produce_vip, &s);
    
    sem_wait(&s.main_blocker);
    sem_wait(&s.main_blocker);
}
