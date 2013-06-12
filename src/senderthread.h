#ifndef _SENDERTHREAD_HEADER_
#define _SENDERTHREAD_HEADER_
#include "nmqueue.h"

#define SENDERTHREAD_NODATA 1
#define SENDERTHREAD_DATA   0

/*! Callback function pointer for sending thread.
 *  A return value of 0 indicates that the message should be send */
typedef int (*sender_source_t)(source_t*, void**, size_t*, void*); 

typedef struct
{
    pthread_t        thread;          /*!< Thread */
    sender_source_t  dataSource;      /*!< Callback */
    void *           dataSourceParam; /*!< Parameter to callback */
    nmqueue_t*       queue;           /*!< Queue */
    volatile int     terminated;      /*!< Indicates the thread should shutdown */
} senderthread_t;

/*!
 * \brief Create a sending thread.
 * 
 * Create a sending thread. dataSource will be called for new data to send.
 * 
 * \param senderThread    Pointer to an uninitialized senderthread_t
 * \param queue           Pointer to an initialized nmqueue_t
 * \param dataSource      Callback
 * \param dataSourceParam Data passed to callback
 * \return                0 on success, 1 on error
 */
int initializeSender(senderthread_t* senderThread,
                     nmqueue_t*      queue,
                     sender_source_t dataSource,
                     void*           dataSourceParam);

/*!
 * \brief Destroy sending thread.
 * 
 * \param senderThread Pointer to initialized senderthread_t
 */
void finalizeSender(senderthread_t* senderThread);

#endif
