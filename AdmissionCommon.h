#ifndef ADMISSION_COMMON_H
#define ADMISSION_COMMON_H

#include <stdint.h> /* uint16_t */

#ifdef DEBUG
#include <ctype.h>   /* isprint */
#include <syscall.h> /* syscall(SYS_gettid) */
#include <unistd.h>  /* syscall(SYS_gettid) */
    /**
     * prints a DEBUG statement
     */
#   define DEBUG_LOG(format, ...) do { \
    atomicPrintf("DEBUG(%ld): %s - " format "\n", \
                 syscall(SYS_gettid), \
                 __func__, \
                 ##__VA_ARGS__); \
} while (0)
    /**
     * examines a string
     */
#   define DEBUG_STRING(string, length, format, ...) do { \
    flockfile(stdout); \
    (void) printf("DEBUG(%ld): %s - " format ": \"", \
                 syscall(SYS_gettid), \
                 __func__, \
                 ##__VA_ARGS__); \
    const char   *str       = (string); \
    const size_t  strLength = (length); \
    size_t i = 0; \
    for (; i < strLength; ++i) { \
        char letter = str[i]; \
        if (isprint(letter)) { \
            (void) putc_unlocked(str[i], stdout); \
        } else { \
            (void) printf("0x%02x", (unsigned int) letter); \
        } \
    } \
    (void) putc_unlocked('"',  stdout); \
    (void) putc_unlocked('\n', stdout); \
    funlockfile(stdout); \
} while (0)
#else
    /**
     * disables DEBUG_LOG()
     */
#   define DEBUG_LOG(...)
    /**
     * disables DEBUG_STRING()
     */
#   define DEBUG_STRING(...)
#endif /* ifdef DEBUG */

struct sockaddr_in; /* forward declaraction */

/**
 * the port number of the Admission server
 */
extern const uint16_t ADMISSION_PORT_NUMBER;

/**
 * @brief atomically print and flush the supplied arguments to stdout
 * @param[in] format the format string
 * @param[in] ... the arguments to be formatted
 */
void atomicPrintf(const char *format, ...);

/**
 * @brief open an IPv4 socket of type @p type
 * @param[in] type the socket type
 * @return a valid, opened socket
 */
int createSocket(int type);

/**
 * TODO
 */
void bindToLoopback(int      sockFd,
                    uint16_t port);

/**
 * @brief get the address associated with the socket @p sockFd
 * @param[in]  sockFd the socket
 * @param[out] address the socket's address
 */
void getAddress(int                 sockFd,
                struct sockaddr_in *address);

/**
 * @brief get the IPv4 address for this @p sockFd
 * @param[in] sockFd the socket
 * @return the IPv4 address associated with the socket in network byte order
 */
uint32_t getIp(int sockFd);

/**
 * @brief print a message to stdout of the form:
 *     <name> has TCP port <port> and IP address <ip><trailer>
 *     where 'port' and 'ip' are the @p socket's TCP port and IPv4 address
 *     respectively
 * @param[in] name    the name of the connection
 * @param[in] trailer the string printed after the IP address
 * @param[in] sockFd  the TCP connection socket
 */
void announceSocket(const char *name,
                    const char *trailer,
                    int         sockFd);

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

/**
 * @brief recv() a 16-bit short off of a @p sockFd
 * @param[in]  sockFd  the socket
 * @param[out] integer the result
 * @return non-zero if successful
 */
int receiveShort(int       sockFd,
                 uint16_t *integer);

/**
 * @brief recv() a string off of a @p sockFd packed accordingly:
 *     <length:uint16_t>
 *     <byte_1>
 *     <byte_2>
 *     ... 
 *     <byte_length>
 * @param[in]  sockFd the socket
 * @param[out] string the result
 * @return non-zero if successful
 */
int receiveString(int    sockFd,
                  char **string);

#endif /* ifndef ADMISSION_COMMON_H */
