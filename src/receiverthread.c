#include "receiverthread.h"

/* Entry point for receiver thread */
/* Will forward received data until shutdown */
static void * receiverProc(void * receiverT)
{
    receiverthread_t* receiverThread=(receiverthread_t*) receiverT;

    while( !receiverThread->terminated )
    {
        source_t source;
        void*    data;
        size_t   dataSize;

        int err;

        if( (err=nmqueue_receive(receiverThread->queue,
                                 &source,
                                 &data,
                                 &dataSize,
                                 receiverThread
                                )) == 0 )
        {
            (*receiverThread->receiverDest)(source, data, dataSize, receiverThread->receiverDestParam);
        }

    }

    return NULL;

}

int initializeReceiver(receiverthread_t* receiverThread,
                       nmqueue_t*        queue,
                       receiver_dest_t   receiverDest,
                       void*             receiverDestParam)
{
    receiverThread->queue        = queue;
    receiverThread->terminated   = 0;

    receiverThread->receiverDest      = receiverDest;
    receiverThread->receiverDestParam = receiverDestParam;

    if( pthread_create( &receiverThread->thread, NULL, receiverProc, receiverThread ) != 0 )
    {
        return 1;
    }
    return 0;
}

void finalizeReceiver(receiverthread_t* receiverThread)
{
    receiverThread->terminated = 1;

    nmqueue_abort( receiverThread->queue, receiverThread );

    pthread_join( receiverThread->thread, NULL );

    return;
}
