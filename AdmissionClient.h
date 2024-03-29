#ifndef ADMISSION_CLIENT_H
#define ADMISSION_CLIENT_H

#include <stdio.h>  /* FILE */
#include <stdint.h> /* uint16_t */

/**
 * Code common to clients of the Admission server.
 */

/**
 * @brief open a TCP connection to the Admission server
 * @param[in] client  the name of the client
 * @param[in] trailer a string that will be appended to the announceSocket()
 *     message called internally
 * @return a socket handle to the Admission server
 */
int connectToAdmission(const char *client,
                       const char *trailer);

/**
 * @brief opens a UDP socket and bind()s it to the loopback address and given
 *     port
 * @param[in] port the port to be bind()ed to
 */
int openAdmissionListener(uint16_t port);

/**
 * TODO
 */
void announceAdmissionListener(const char *client,
                               int         listener);

/**
 * TODO
 */
uint16_t peekShort(int sockFd);

/**
 * @brief read a string packet from a UDP socket @p listener
 */
int listenForString(int    listener,
                    char **string);

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

#endif /* ifndef ADMISSION_CLIENT_H */
