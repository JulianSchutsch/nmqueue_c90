#include "senderthread.h"

/* Thread entry point for sending thread */
/* Will keep checking wether data is available for sending until shutdown */
static void* senderProc(void * senderT)
{
    senderthread_t* senderThread=(senderthread_t*)senderT;

    while( !senderThread->terminated )
    {

        source_t source;
        void*    data;
        size_t   dataSize;
        if( (*senderThread->dataSource)(&source, &data, &dataSize, senderThread->dataSourceParam) == 0 )
        {
            /* Send message. Return value ignored since NMQUEUEERROR_ABORT will be indicated by
             * senderThread->terminated as well. */
            nmqueue_send(senderThread->queue, source, data, dataSize, senderThread);
        }

    }

    return NULL;

}

int initializeSender(senderthread_t* senderThread,
                     nmqueue_t*      queue,
                     sender_source_t dataSource,
                     void*           dataSourceParam)
{
    senderThread->queue      = queue;
    senderThread->terminated = 0;

    senderThread->dataSource      = dataSource;
    senderThread->dataSourceParam = dataSourceParam;

    if ( pthread_create( &senderThread->thread, NULL, senderProc, senderThread ) != 0 )
    {
        return 1;
    }

    return 0;
}

void finalizeSender(senderthread_t* senderThread)
{
    senderThread->terminated = 1;
    nmqueue_abort( senderThread->queue, senderThread );

    pthread_join( senderThread->thread, NULL );
}
