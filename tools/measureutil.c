#include "measureutil.h"

#include <math.h>

uint64_t mu_deviation(int64_t* data,
                      size_t   count)
{
    size_t  i;
    int64_t sqrsum = 0;
    int64_t mean   = mu_mean( data, count );
    for( i = 0; i < count ; ++i )
    {
        sqrsum = sqrsum + (data[i] - mean)*(data[i] - mean);
    }
    return sqrt( sqrsum/count );
}

uint64_t mu_mean(int64_t* data,
                 size_t   count)
{
    size_t  i;
    int64_t sum = 0;

    for( i = 0; i < count ; ++i )
    {
        sum = sum + data[i];
    }

    return sum/count;
}

void mu_write(FILE*    file,
              int64_t* data,
              size_t   count)
{
    size_t i;
    for( i = 0; i < count ; i++ )
    {
        fprintf( file, "%li\n", data[i] );
    }
}
