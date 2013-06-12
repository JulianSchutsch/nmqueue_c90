#include "timespecutil.h"

uint64_t timespec_to_us(struct timespec time)
{
    return time.tv_sec*1000*1000+time.tv_nsec/1000;
}

struct timespec us_to_timespec(uint64_t time)
{
    struct timespec result;
    result.tv_sec  = time/(1000*1000);
    result.tv_nsec = (time % (1000*1000))*1000;

    return result;
}
