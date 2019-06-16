#ifndef ADMISSION_CLIENT_H
#define ADMISSION_CLIENT_H

#include <stdio.h>  /* FILE */
#include <stdint.h> /* uint16_t */


int connectToAdmission(const char *client,
                       const char *trailer);

/**
 * @brief read a config line of the form <key><delimiter><value> from @p input
 * @param[in,out] input     the file containing the line
 * @param[in]     delimiter the delimiter character
 * @param[out]    key       the free()-able value read from @input
 * @param[out]    value     the free()-able value read from @input
 * @return non-zero if successful
 */
int readConfig(FILE  *input,
               char   delimiter,
               char **key,
               char **value);

char *packShort(char     *buffer,
                uint16_t  integer);

char *packString(char       *buffer,
                 const char *string,
                 uint16_t    length);

#endif /* ifndef ADMISSION_CLIENT_H */
