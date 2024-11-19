
// Aria Kafie 828109926
// Anosh Taraporevala 827939860

#include <cstdlib>
#include <unistd.h>

#include "consumer.h"
#include "log.h"
#include "producer.h"

// define some constants
#define DEFAULT_MAX_REQUESTS 120
#define DEFAULT_TX_TIME      0
#define DEFAULT_R9_TIME      0
#define DEFAULT_GENERAL_TIME 0
#define DEFAULT_VIP_TIME     0

int main(int argc, char **argv)
{
    // declare some variables to hold command line arguments
    int s = DEFAULT_MAX_REQUESTS;
    int x = DEFAULT_TX_TIME;
    int r = DEFAULT_R9_TIME;
    int g = DEFAULT_GENERAL_TIME;
    int v = DEFAULT_VIP_TIME;

    // parse the command line arguments
    for (char opt; (opt = getopt(argc, argv, "s:x:r:g:v:")) != -1;)
        switch (opt)
        {
            case 's': s = std::atoi(optarg); break;
            case 'x': x = std::atoi(optarg); break;
            case 'r': r = std::atoi(optarg); break;
            case 'g': g = std::atoi(optarg); break;
            case 'v': v = std::atoi(optarg); break;
        }

    // initialize the shared data
    SharedData sd(s, x, r, g, v);

    // declare producer and consumer threads
    pthread_t general_producer, vip_producer, tx, r9;

    // create each thread
    pthread_create(&general_producer, NULL, &producer<GeneralTable>, &sd);
    pthread_create(&vip_producer,     NULL, &producer<VIPRoom>,      &sd);
    pthread_create(&tx,               NULL, &consumer<TX>,           &sd);
    pthread_create(&r9,               NULL, &consumer<Rev9>,         &sd);

    // sleep until the consumers are done
    sem_wait(&sd.main_blocker);

    // store consumption data in an array
    unsigned int *consumed[RequestTypeN] = { sd.consumed[TX], sd.consumed[Rev9] };

    // log a summary
    output_production_history(sd.produced, consumed);
}
