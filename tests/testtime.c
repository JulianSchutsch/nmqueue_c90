/* Test program to measure the time required for a message to be sent. */

#include "src/nmqueue.h"
#include "src/receiverthread.h"
#include "src/senderthread.h"

#include "tools/measureutil.h"
#include "tools/timespecutil.h"

#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

nmqueue_t queue;

volatile int started; /* Flag used to have a common starting point */

/* Data passed to sending threads */
typedef struct
{
    source_t source; /* Unique source id for each sending threads */
    size_t   index;  /* Number of sent messages */
    size_t   count;  /* Number of message to be send */
    int64_t* start;  /* Array: Time point where each message was sent */
    int64_t* delta;  /* Array: Time it took for each message to be send */
    struct timespec senddelta; /* Time between two sends */
} producerdata_t;

/* Data passed to receiving threads */
typedef struct
{
    long     count; /* Number of messages received */
} consumerdata_t;

/* Data array for all sending threads */
producerdata_t* producerData;
/* Data array for all receiving threads */
consumerdata_t* consumerData;

/* Callback called by the sending thread */
int producer(source_t* source,
             void**    data,
             size_t*   dataSize,
             void*     param)
{
    producerdata_t* pdata = (producerdata_t*)param;

    /* Data left to send? */
    if( pdata->index != pdata->count )
    {
        /* Wait for a common starting point */
        while(!started)
        {
            sched_yield();
        }

        /* Delay */
        {
            struct timespec remaining = pdata->senddelta;
            while( ( remaining.tv_sec != 0 || remaining.tv_nsec != 0 ) &&
                   nanosleep( &remaining , &remaining ) != 0 );
        }

        *source   = pdata->source;
        *data     = NULL;
        *dataSize = pdata->index;

        /* Set time sent point */
        {
            struct timespec starttime;
            clock_gettime( CLOCK_MONOTONIC, &starttime );
            pdata->start[pdata->index] = timespec_to_us( starttime );
        }

        pdata->index++;

        return 0;
    }
    else
    {
        sched_yield();
    }

    return 1;
}

/* Callback called by the receiving thread */
void consumer(source_t source,
              void*    data,
              size_t   dataSize,
              void*    param)
{
    struct timespec stoptime;
    consumerdata_t* cdata = (consumerdata_t*)param;

    /* Set time received-time sent */
    clock_gettime( CLOCK_MONOTONIC, &stoptime );
    producerData[source].delta[dataSize] = timespec_to_us( stoptime )-producerData[source].start[dataSize];

    /* Count received message */
    cdata->count++;
}

/* Get the number of message received by all threads */
long getTotalConsumed(unsigned int consumers)
{
    int i;
    long count=0;

    for(i=0;i<consumers;++i)
    {
        count+=consumerData[i].count;
    }
    return count;
}

/*
 * Test procedure for measuring the time required for sending messages.
 */
void testnm(unsigned int n, /*< n sending threads */
            unsigned int m, /*< m receiving threads */
            long count,     /*< number of message sent by each sending thread */
            uint64_t delta) /*< time delay between each message sent (for one thread) */
{

    int i;

    /* Allocate test data structures */
    senderthread_t*   senders   = (senderthread_t*)malloc(n*sizeof(senderthread_t));
    receiverthread_t* receivers = (receiverthread_t*)malloc(m*sizeof(receiverthread_t));
    producerData                = (producerdata_t*)malloc(n*sizeof(producerdata_t));
    consumerData                = (consumerdata_t*)malloc(m*sizeof(consumerdata_t));
    
    started = 0;

    for( i=0 ; i<n ; ++i )
    {
        producerData[i].count     = count;
        producerData[i].source    = i;
        producerData[i].start     = (int64_t*)malloc(count*sizeof(int64_t));
        producerData[i].delta     = (int64_t*)malloc(count*sizeof(int64_t));
        producerData[i].senddelta = us_to_timespec( delta );
        producerData[i].index     = 0;
    }

    for( i=0 ; i<m ; ++i )
    {
        consumerData[i].count = 0;
    }

    /* Create sending threads */
    for( i=0 ; i<n ; ++i )
    {
        initializeSender( &senders[i], &queue, producer, &producerData[i] );
    }

    /* Create receiving threads */
    for( i=0 ; i<m ; ++i )
    {
        initializeReceiver( &receivers[i], &queue, consumer, &consumerData[i] );
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

    printf("Distribution of messages among receiver threads:\n");
    for( i = 0 ; i < m ; ++i )
    {
        printf("Receiver-Thread %i received %lu messages.\n", i, consumerData[i].count);
    }

    for( i = 0; i < n ; ++i )
    {
        int64_t meanDelta = 0;
        int64_t devDelta  = 0;
        meanDelta = mu_mean( producerData[i].delta, producerData[i].count );
        devDelta  = mu_deviation( producerData[i].delta, producerData[i].count);
        printf("Message send time average %i delta %li µs +- %li\n", i, (long int)meanDelta, (long int)devDelta );
    }
    
    for( i=0 ; i<n ; ++i )
    {
        finalizeSender( &senders[i] );
    }

    for( i=0 ; i<m ; ++i )
    {
        finalizeReceiver( &receivers[i] );
    }

    for( i=0 ; i<n ; ++i )
    {
        free( producerData[i].start );
        free( producerData[i].delta );
    }

    free(consumerData);
    consumerData = NULL;

    free(producerData);
    producerData = NULL;

    free( senders );
    free( receivers );
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

    testnm (1, 1, 1000000, 0 );
    testnm( 2, 2, 1000000, 0 );

    nmqueue_finalize(&queue);
    return 0;
}
