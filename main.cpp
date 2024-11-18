
#include <sstream>
#include <string>

#include "consumer.h"
#include "log.h"
#include "producer.h"

int main(int argc, char **argv)
{
    SharedData sd(100);
    pthread_t general_producer, vip_producer, tx, r9;

    pthread_create(&general_producer, NULL, &producer<GeneralTable>, &sd);
    pthread_create(&vip_producer,     NULL, &producer<VIPRoom>,      &sd);
    pthread_create(&tx,               NULL, &consumer<TX>,           &sd);
    pthread_create(&r9,               NULL, &consumer<Rev9>,         &sd);
    
    sem_wait(&sd.main_blocker);

    unsigned int *consumed[RequestTypeN] = { sd.consumed[TX], sd.consumed[Rev9] };
    
    output_production_history(sd.produced, consumed);
}
