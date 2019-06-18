#ifndef ADMISSION_COMMON_H
#define ADMISSION_COMMON_H

#include <stdint.h> /* uint16_t */

/**
 * the port number of the Admission server
 */
extern const uint16_t ADMISSION_PORT_NUMBER;

/**
 * @brief print a message to stdout of the form:
 *     <name> has TCP port <port> and IP address <ip><trailer>
 *     where 'port' and 'ip' are the @p socket's TCP port and IPv4 address
 *     respectively
 * @param[in] name    the name of the connection
 * @param[in] trailer the string printed after the IP address
 * @param[in] socket  the TCP connection socket
 */
void announceConnection(const char *name,
                        const char *trailer,
                        int         socket);

#endif /* ifndef ADMISSION_COMMON_H */
