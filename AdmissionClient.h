#ifndef ADMISSION_CLIENT_H
#define ADMISSION_CLIENT_H

#include <stdio.h>  /* FILE */
#include <stdint.h> /* uint16_t */

/**
 * Code common to clients of the Admission server.
 */

/**
 * @brief open a TCP connection to the Admission server
 * @param client  the name of the client
 * @param trailer a string that will be appended to the announceConnection()
 *     message called internally
 * @return a socket handle to the Admission server
 */
int connectToAdmission(const char *client,
                       const char *trailer);

/**
 * @brief read a config line of the form <key><delimiter><value> from @p input
 * @param[in]  input     the file containing the line
 * @param[in]  delimiter the delimiter character
 * @param[out] key       the free()-able value read from @input
 * @param[out] value     the free()-able value read from @input
 * @return non-zero if successful
 */
int readConfig(FILE  *input,
               char   delimiter,
               char **key,
               char **value);

/**
 * @brief copy @p integer into buffer
 * @param[in] buffer  the destination buffer
 * @param[in] integer the value to be copied
 * @return a pointer past the end of the written @p integer
 */
char *packShort(char     *buffer,
                uint16_t  integer);

/**
 * @brief copy the variable-length @p string into buffer.  The provided @p
 *     length will be written first, followed by @p length bytes of @p string.
 * @param[in] buffer  the destination buffer
 * @param[in] string the value to be copied
 * @param[in] length the length of the provided string
 * @return a pointer past the end of the written @p string
 */
char *packString(char       *buffer,
                 const char *string,
                 uint16_t    length);

#endif /* ifndef ADMISSION_CLIENT_H */
