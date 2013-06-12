/* Demo program for the multi sender multi receiver queue.
 * 
 * The actual relevant source is in nmqueue.
 * Important tools are in receiverthread and senderthread.
 * 
 * Julian Schutsch
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "src/nmqueue.h"
#include "src/senderthread.h"
#include "src/receiverthread.h"

#define SENDERS 10
#define RECEIVERS 20
#define BUFFERSIZE 1024

nmqueue_t queue;
senderthread_t   senderThreads[SENDERS];
receiverthread_t receiverThreads[RECEIVERS];

/* Make believe source */
int get_external_data(char * buffer, int bufferSizeInBytes)
{
    sleep(1); /* Slow down the process to watch */
    printf("External...%p\n", buffer );
    return 0;
}

/* Make believe sink */
void process_data(char * buffer, int bufferSizeInBytes)
{
    printf("Process...%p\n", buffer );
    return;
}

/* Sender thread callback */
int producer(source_t* source,
             void**    data,
             size_t*   dataSize,
             void*     param)
{
    /* Allocate data buffer */
    *source   = (int)((size_t)(param));
    *data     = malloc( BUFFERSIZE );
    *dataSize = BUFFERSIZE;

    /* It is certainly possible to avoid allocation of memory,
     * but this depends a lot on the situation and may
     * result in a custom memory manager.
     * Risk is high of reinventing the wheel.*/

    /* Get data from external source */
    if( get_external_data( (char*)(*data) , BUFFERSIZE ) != 0 )
    {
        /* Release data buffer */
        free( *data );
        return SENDERTHREAD_NODATA; /* Do not send data,
                                       but let the thread loop check for termination */
    }

    return SENDERTHREAD_DATA; /* Send data */
}

/* Receiver thread callback */
void consumer(source_t source,
             void*     data,
             size_t    dataSize,
             void*     param)
{
    printf("Consume from %i in %i\n",source,(int)((size_t)(param)));
    process_data( (char*)data, dataSize );
    free( data );
}

int main(int argc, char** argv)
{
    size_t i;

    nmqueue_initialize(&queue , 1024); /* Create a bounded message queue of 1024 messages. */

    for( i=0 ; i<SENDERS ; ++i )
    {
        if( initializeSender( &senderThreads[i], &queue, producer, (void*)i ) != 0 )
        {
            perror("Sender initialize failed\n");
            exit(1);
        }
    }

    for( i=0; i<RECEIVERS ; ++i )
    {
        if( initializeReceiver( &receiverThreads[i], &queue, consumer, (void*)i ) != 0 )
        {
            perror("Receiver initialize failed\n");
            exit(1);
        }
    }
    
    for(;;)
    {
        sleep(1);
    }

    /* Dead code, since no condition for program termination exists yet */

    for( i=0 ; i<SENDERS ; ++i )
    {
        finalizeSender( &senderThreads[i] );
    }

    for( i=0; i<RECEIVERS ; ++i )
    {
        finalizeReceiver( &receiverThreads[i] );
    }

    nmqueue_finalize(&queue);

    return 0;
}
