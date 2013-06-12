/* Test program to check wether messages are correctly delivered */
#include "src/nmqueue.h"
#include "src/receiverthread.h"
#include "src/senderthread.h"

#include "tools/timespecutil.h"

#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#define PRODUCER_COUNT 20
#define CONSUMER_COUNT 20

nmqueue_t queue;

volatile int started; /* Used to get a common starting point */

/* Data passed to sending threads */
typedef struct
{
    source_t source;              /* Unique source id for a producer */
    long     count;               /* Number of remaining messages to be send */
    pthread_mutex_t arrivedMutex; /* Mutex to ensure correct counting of
                                   * arrived messages */
    int*     arrived;             /* Array to count arrived messages for each
                                   * sent message */
} producerdata_t;

/* Data passed to receiving threads */
typedef struct
{
    long     count; /* Number of received messages by a thread */
} consumerdata_t;

/* Data array for all sending threads */
producerdata_t* producerData;
/* Data array for all receiving threads */
consumerdata_t* consumerData;

/* Callback for sending thread */
int producer(source_t* source,
             void**    data,
             size_t*   dataSize,
             void*     param)
{
    producerdata_t* pdata = (producerdata_t*)param;
    
    /* Any more message to be sent? */
    if( pdata->count != 0 )
    {
        while(!started)
        {
            sched_yield();
        }

        pdata->count--; /* One message less to send */

        *source   = pdata->source; /* Own source */
        *data     = NULL;
        *dataSize = pdata->count;  /* Use dataSize as a message Id. */

        return 0;
    }
    else
    {
        sched_yield();
    }

    return 1;
}

/* Callback for receiving thread */
void consumer(source_t source,
              void*    data,
              size_t   dataSize,
              void*    param)
{
    consumerdata_t* cdata = (consumerdata_t*)param;

    /* Count received message using dataSize as message Id. */
    pthread_mutex_lock(&producerData[source].arrivedMutex);
    producerData[source].arrived[dataSize]++;
    pthread_mutex_unlock(&producerData[source].arrivedMutex);

    cdata->count++; /* One more message received */

}

/* Get the number of messages received by all threads */
long getTotalConsumed(unsigned int consumers)
{
    int  i;
    long count = 0;

    for( i=0 ; i<consumers ; ++i )
    {
        count+=consumerData[i].count;
    }

    return count;
}

/*
 * Test procedure to check if all messages are sent and only sent once.
 */
void testnm(unsigned int n,
            unsigned int m,
            long count)
{

    int i;

    /* Allocate memory */
    senderthread_t*   senders      = (senderthread_t*)malloc(n*sizeof(senderthread_t));
    receiverthread_t* receivers    = (receiverthread_t*)malloc(m*sizeof(receiverthread_t));
    producerData                   = (producerdata_t*)malloc(n*sizeof(producerdata_t));
    consumerData                   = (consumerdata_t*)malloc(m*sizeof(consumerdata_t));
    
    started = 0;

    for( i=0 ; i<n ; ++i )
    {
        producerData[i].count   = count;
        producerData[i].source  = i;
        producerData[i].arrived = (int*)malloc(count*sizeof(int));

        memset(producerData[i].arrived, 0, count*sizeof(int));
        pthread_mutex_init(&producerData[i].arrivedMutex, NULL);
    }

    for( i=0; i<m ; ++i )
    {
        consumerData[i].count = 0;
    }

    /* Create sending threads */
    for( i=0 ; i<n ; ++i )
    {
        initializeSender(&senders[i], &queue, producer, &producerData[i]);
    }

    /* Create receiving threads */
    for( i=0 ; i<m ; ++i )
    {
        initializeReceiver(&receivers[i], &queue, consumer, &consumerData[i]);
    }

    /* Run test */
    started = 1;
    {
        struct timespec starttime;
        struct timespec stoptime;
        int64_t delta;
        clock_gettime( CLOCK_MONOTONIC, &starttime );

        do
        {
            sched_yield();
        } while(getTotalConsumed(m)<n*count);

        clock_gettime( CLOCK_MONOTONIC, &stoptime );

        delta = timespec_to_us( stoptime )-timespec_to_us( starttime );

        printf("Total time consumed %li µs\n", (long int)delta);

    }
    started = 0;

    /* Verify if all messages have been delivered exactly once */
    for( i=0 ; i<n ; ++i )
    {
        int j;
        for(j=0; j<count;++j)
        {
            if(producerData[i].arrived[j]!=1)
            {
                printf("Invalid arrival count for source %i, message %i : %i\n",i,j,producerData[i].arrived[j]);
            }
        }
    }

    printf("Processing distribution among receiver threads:\n");
    for( i=0 ; i<m ; ++i )
    {
        printf("Receiver-Thread %i  received %lu\n", i, consumerData[i].count);
    }
    
    /* Destroy sending threads */
    for( i=0 ; i<n ; ++i )
    {
        finalizeSender( &senders[i] );
    }

    /* Destroy receiving threads */
    for( i=0 ; i<m ; ++i )
    {
        finalizeReceiver( &receivers[i] );
    }

    /* Release memory and mutexes */
    for( i=0 ; i<n ; ++i )
    {
        free(producerData[i].arrived);
        pthread_mutex_destroy(&producerData[i].arrivedMutex);
    }

    free(consumerData);
    consumerData=NULL;

    free(producerData);
    producerData=NULL;

    free(senders);
    free(receivers);
}

int main()
{
    {
        int err;
        if( (err=nmqueue_initialize(&queue,1024)) != NMQUEUEERROR_NOERROR)
        {
            printf("Failed to initialize nmqueue: %s\n",nmqueue_error_to_string(err));
        }
    }

    testnm(PRODUCER_COUNT, CONSUMER_COUNT, 1000000);

    nmqueue_finalize(&queue);

    return 0;
}
