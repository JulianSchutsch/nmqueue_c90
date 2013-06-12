#include "nmqueue.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define NMQUEUE_INVARIANT(queue)\
    assert( queue->queue != NULL );\
    assert( queue->readPosition < queue->length );\
    assert( queue->writePosition < queue->length );

static const char* errors[] = {
    "No Error",
    "Mutex initialize failed",
    "Conditional variable initialize failed",
    "Out of memory",
    "Abort signal catched"};

static const char* invalidError = "Invalid error";

const char* nmqueue_error_to_string(int err)
{
    if( err<0 || err>NMQUEUEERROR_MAX )
    {
        return invalidError;
    }
    return errors[err];
}

void nmqueue_abort(nmqueue_t* queue,
                   void*      threadId)
{
    assert( queue != NULL );
    NMQUEUE_INVARIANT( queue )

    pthread_mutex_lock( &queue->mutex );

    queue->abort = threadId;

    /* Wakeup both sending and receiving threads */
    pthread_cond_broadcast( &queue->writtenCond );
    pthread_cond_broadcast( &queue->readCond );

    pthread_mutex_unlock( &queue->mutex );

}

int nmqueue_initialize(nmqueue_t* queue,
                       size_t     length)
{
    assert( queue != NULL );
    assert( length != 0 );

    queue->writePosition = 0;
    queue->readPosition  = 0;
    queue->length        = length;
    queue->queue         = (struct nmqueue_message_s*)malloc( length*sizeof(struct nmqueue_message_s) );
    queue->abort         = (void*)(1);

    if( queue->queue == NULL )
    {
        return NMQUEUEERROR_OUTOFMEMORY;
    }

    /* Initialize mutex and conditional variables for both read and written */
    {
        int error;

        if ( pthread_mutex_init( &queue->mutex, NULL ) == 0 )
        {

            if ( pthread_cond_init( &queue->writtenCond, NULL ) == 0 )
            {

                if ( pthread_cond_init( &queue->readCond, NULL ) == 0 )
                {
                    NMQUEUE_INVARIANT(queue);
                    return NMQUEUEERROR_NOERROR;
                }

                error = NMQUEUEERROR_CONDVAR_INITIALIZE_FAILED;

                pthread_cond_destroy( &queue->writtenCond );

            } else { error = NMQUEUEERROR_CONDVAR_INITIALIZE_FAILED; }

            pthread_mutex_destroy( &queue->mutex );

        } else { error = NMQUEUEERROR_MUTEX_INITIALIZE_FAILED; }

        free( queue->queue );
        return error;

    }

}

void nmqueue_finalize(nmqueue_t* queue)
{
    /* Carefull, threads active? TODO*/
    assert( queue != NULL);
    NMQUEUE_INVARIANT( queue );

    pthread_cond_destroy( &queue->writtenCond );
    pthread_cond_destroy( &queue->readCond );
    pthread_mutex_destroy( &queue->mutex );

    /* Actually the queue shouldn't be freed if there is still data to be processed,
       but in case there still is, at least the entries are deleted.
       
       It is impossible to make assumptions about the data pointers since
       they may not be allocated or may not even be used as pointers. => LEAK WARNING*/

    free( queue->queue );

}

int nmqueue_send(nmqueue_t* queue,
                 source_t   source,
                 void*      data,
                 size_t     dataSize,
                 void*      threadId)
{
    assert( queue != NULL );
    NMQUEUE_INVARIANT( queue );

    pthread_mutex_lock( &queue->mutex );

    /* aborted thread? */
    if( queue->abort == threadId )
    {
        printf("ABB:%p\n",queue->abort);
        queue->abort = NULL;
        pthread_mutex_unlock(&queue->mutex);
        return NMQUEUEERROR_ABORT;
    }

    /* Wait until writting is possible */
    while ((queue->writePosition+1) % queue->length == queue->readPosition)
    {
        pthread_cond_wait(&queue->readCond, &queue->mutex);

        /* aborted thread? */
        if( queue->abort == threadId )
        {
            queue->abort = NULL;
            pthread_mutex_unlock(&queue->mutex);
            return NMQUEUEERROR_ABORT;
        }
    }

    /* Write message */
    {
        struct nmqueue_message_s* message = &queue->queue[ queue->writePosition ];

        message->source   = source;
        message->data     = data;
        message->dataSize = dataSize;
    }

    queue->writePosition = (queue->writePosition+1) % queue->length;
    NMQUEUE_INVARIANT( queue );

    pthread_cond_signal(&queue->writtenCond);

    pthread_mutex_unlock(&queue->mutex);

    return NMQUEUEERROR_NOERROR;

}

int nmqueue_receive(nmqueue_t* queue,
                    source_t*  source,
                    void**     data,
                    size_t*    dataSize,
                    void*      threadId)
{
    assert( queue    != NULL );
    assert( source   != NULL );
    assert( data     != NULL );
    assert( dataSize != NULL );
    NMQUEUE_INVARIANT( queue );

    pthread_mutex_lock( &queue->mutex );

    /* aborted thread? */
    if( queue->abort == threadId)
    {
        queue->abort = NULL;
        pthread_mutex_unlock(&queue->mutex);
        return NMQUEUEERROR_ABORT;
    }

    /* Wait until reading is possible */
    while( queue->readPosition == queue->writePosition )
    {

        pthread_cond_wait( &queue->writtenCond, &queue->mutex );

        /* aborted thread? */
        if( queue->abort == threadId )
        {
            queue->abort = NULL;
            pthread_mutex_unlock(&queue->mutex);
            return NMQUEUEERROR_ABORT;
        }

    }

    /* Read message */
    {
        struct nmqueue_message_s* message = &queue->queue[ queue->readPosition ];

        *source   = message->source;
        *data     = message->data;
        *dataSize = message->dataSize;

        queue->readPosition = (queue->readPosition+1) % queue->length;

        NMQUEUE_INVARIANT( queue );

        pthread_cond_signal(&queue->readCond);

    }

    pthread_mutex_unlock(&queue->mutex);

    return NMQUEUEERROR_NOERROR;

}
