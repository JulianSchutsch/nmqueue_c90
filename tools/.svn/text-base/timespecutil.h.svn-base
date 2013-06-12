#ifndef _TIMESPECUTIL_HEADER_
#define _TIMESPECUTIL_HEADER_

#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

#define TIMESPECADD( left, right ) do{      \
                                            \
        left.tv_sec  += right.tv_sec;       \
        left.tv_nsec += right.tv_nsec;      \
                                            \
        if( left.tv_nsec>1000*1000*1000 )   \
        {                                   \
            left.tv_sec++;                  \
            left.tv_nsec -= 1000*1000*1000; \
        }                                   \
                                            \
    } while(0);

#define TIMESPECSUB( left, right ) do{      \
                                            \
        left.tv_sec -= right.tv_sec;        \
        if( left.tv_nsec < right.tv_nsec ){ \
            left.tv_sec--;                  \
            left.tv_nsec+=1000*1000*1000;   \
        }                                   \
        left.tv_nsec -= right.tv_nsec;      \
                                            \
    } while(0);

#define TIMESPECCMP( left, right, op )      \
                                            \
   (left.tv_sec == right.tv_sec?            \
        left.tv_nsec op right.tv_nsec:      \
        left.tv_sec op right.tv_sec         \
   )

/* Truncates nanoseconds, no rounding */
uint64_t timespec_to_us(struct timespec time);
struct timespec us_to_timespec(uint64_t time);

#endif
