
#include <cstdlib>
#include <unistd.h>

#include "consumer.h"
#include "log.h"
#include "producer.h"

#define DEFAULT_MAX_REQUESTS 120
#define DEFAULT_TX_TIME      0
#define DEFAULT_R9_TIME      0
#define DEFAULT_GENERAL_TIME 0
#define DEFAULT_VIP_TIME     0

int main(int argc, char **argv)
{
    int s = DEFAULT_MAX_REQUESTS;
    int x = DEFAULT_TX_TIME;
    int r = DEFAULT_R9_TIME;
    int g = DEFAULT_GENERAL_TIME;
    int v = DEFAULT_VIP_TIME;
    
    for (char opt; (opt = getopt(argc, argv, "s:x:r:g:v:")) != -1;)
        switch (opt)
        {
            case 's': s = std::atoi(optarg); break;
            case 'x': x = std::atoi(optarg); break;
            case 'r': r = std::atoi(optarg); break;
            case 'g': g = std::atoi(optarg); break;
            case 'v': v = std::atoi(optarg); break;
        }
    
    SharedData sd(s, x, r, g, v);
    
    pthread_t general_producer, vip_producer, tx, r9;

    pthread_create(&general_producer, NULL, &producer<GeneralTable>, &sd);
    pthread_create(&vip_producer,     NULL, &producer<VIPRoom>,      &sd);
    pthread_create(&tx,               NULL, &consumer<TX>,           &sd);
    pthread_create(&r9,               NULL, &consumer<Rev9>,         &sd);
    
    sem_wait(&sd.main_blocker);

    unsigned int *consumed[RequestTypeN] = { sd.consumed[TX], sd.consumed[Rev9] };
    
    output_production_history(sd.produced, consumed);
}
