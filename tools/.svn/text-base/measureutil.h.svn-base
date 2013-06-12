#ifndef _MEASUREUTIL_HEADER_
#define _MEASUREUTIL_HEADER_

#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>

/*!
 * \brief Calculates the mean value of a data array
 * 
 * Calculates a very rough mean of a data array and can handle only a total
 * integral value of max(uint64_t).
 * 
 * \param data  Array
 * \param count Length of the Array
 * \return      Sum ( data ) / count
 */
uint64_t mu_mean(int64_t*  data,
                 size_t    count);

uint64_t mu_deviation(int64_t* data,
                      size_t   count);

void mu_write(FILE*    file,
              int64_t* data,
              size_t   count);

#endif
