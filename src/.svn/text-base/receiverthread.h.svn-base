#ifndef _RECEIVERTHREAD_HEADER_
#define _RECEIVERTHREAD_HEADER_

#include "nmqueue.h"

/*! Callback function pointer for receiving thread. */
typedef void (*receiver_dest_t)(source_t, void*, size_t, void*);

typedef struct
{
    pthread_t       thread;            /*!< Thread */
    receiver_dest_t receiverDest;      /*!< Callback */
    void*           receiverDestParam; /*!< Parameter to callback */
    nmqueue_t*      queue;             /*!< Queue */
    volatile int    terminated;        /*!< Indicates the thread should shutdown */
} receiverthread_t;

/*!
 * \brief Create receiver thread.
 * 
 * Create a receiver thread. receiverDest will be called for every received
 * message.
 * 
 * \param receiverThread     Pointer to uninitialized receiverthread_t
 * \param queue              Pointer to initialized nmqueue_t
 * \param receiverDest       Callback
 * \param receiverDestParam  Data passed to callback
 * \return                   0 on success, 1 on error
 */
int initializeReceiver(receiverthread_t* receiverThread,
                       nmqueue_t*        queue,
                       receiver_dest_t   receiverDest,
                       void*             receiverDestParam);

/*!
 * \brief Destroy receiver thread.
 * 
 * \param receiverThread Pointer to initialized receiverthread_t
 */
void finalizeReceiver(receiverthread_t* receiverThread);

#endif
