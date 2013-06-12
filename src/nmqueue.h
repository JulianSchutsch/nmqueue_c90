#ifndef _NMQUEUE_HEADER_
#define _NMQUEUE_HEADER_

#include <pthread.h>

/*! Error numbers */
#define NMQUEUEERROR_NOERROR 0
#define NMQUEUEERROR_MUTEX_INITIALIZE_FAILED 1
#define NMQUEUEERROR_CONDVAR_INITIALIZE_FAILED 2
#define NMQUEUEERROR_OUTOFMEMORY 3
#define NMQUEUEERROR_ABORT 4
#define NMQUEUEERROR_MAX 4

typedef int source_t;

/*! Queue ring buffer entry */
struct nmqueue_message_s
{
    /*! User data, this has no meaning to the nmqueue implementation. */
    void*    data;
    size_t   dataSize;
    source_t source;
};

/*! Queue data structure */
typedef struct
{
   size_t readPosition;             /*!< Ring buffer read position, must be in 0..length-1 */
   size_t writePosition;            /*!< Ring buffer write position, must be in 0..length-1 */
   size_t length;                   /*!< Ring buffer size in elements */
   void*  abort;                    /*!< Pointer to identify the thread to abort */
   struct nmqueue_message_s* queue; /*!< Ring buffer */
   pthread_mutex_t    mutex;        /*!< Mutex, has to be locked for all ring buffer operations */
   pthread_cond_t     writtenCond;  /*!< Condition to be signaled on every write */
   pthread_cond_t     readCond;     /*!< Condition to be signaled on every read */ 
} nmqueue_t;

/*!
 * \brief Initialize message queue.
 * 
 * Initialize message queue and allocate a bounded
 * ring buffer of (->)length bytes.
 * 
 * \param queue  Pointer to a not initialized instance of nmqueue_t
 * \param length Length of the bounded ring buffer, not 0
 * 
 * \return      Error code, ERROR_NOERROR on success.
 */

int nmqueue_initialize(nmqueue_t* queue,
                       size_t     length);

/*!
 * \brief Finalize message queue.
 * 
 * Finalize message queue and free bounded ring buffer.
 * 
 * \param queue Pointer to an initialized instance of nmqueue_t
 */

void nmqueue_finalize(nmqueue_t* queue);

/*!
 * \brief Send abort message to one blocked thread.
 * 
 * Sends an abort message to one blocked thread.
 * This will wakeup every thread working on the queue for a short moment.
 * Will cause the nmqueue_send and nmqueue_receive to return ERROR_ABORT
 * in the specified thread once.
 * 
 * \param queue  Pointer to an initialized instance of nmqueue_t
 */

void nmqueue_abort(nmqueue_t* queue,
                   void*      threadId);

/*!
 * \brief Blocking message send to queue.
 * 
 * A message is inserted in the bounded ring buffer of the message queue.
 * If this buffer is full, send blocks. It waits for a signal from
 * a receiving thread or an abort signal to unblock.
 * An abort signal is indicated by ERROR_ABORT.
 * 
 * \param queue    Pointer to an initialized instance of nmqueue_t
 * \param source   Any source_t
 * \param data     Any void*
 * \param dataSize Any size_t
 * \return         Error code, ERROR_NOERROR on success
 */

int nmqueue_send(nmqueue_t* queue,
                 source_t   source,
                 void*      data,
                 size_t     dataSize,
                 void*      threadId);

/*!
 * \brief Blocking message receive from queue.
 * 
 * The oldest message is taken from the bounded ring buffer of the message queue.
 * If no message is available, receive blocks. It waits for a signal from
 * a sending thread or an abort signal to unblock.
 * An abort signal is indicated by ERROR_ABORT.
 * 
 * \param queue    Pointer to an initialized instance of nmqueue_t
 * \param source   Reference to a source_t
 * \param data     Reference to a void*
 * \param dataSize Refence to a size_t
 * \return         Error code, ERROR_NOERROR on success
 */

int nmqueue_receive(nmqueue_t* queue,
                    source_t*  source,
                    void**     data,
                    size_t*    dataSize,
                    void*      threadId);

/*!
 * \brief Converts a nmqueue error to string.
 * 
 * \param err error code
 * \return    error string
 */

const char* nmqueue_error_to_string(int err);

#endif
