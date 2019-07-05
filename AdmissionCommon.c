#include "AdmissionCommon.h"

#include <sys/types.h>  /* socket */
#include <sys/socket.h> /* getsockname */
#include <arpa/inet.h>  /* inet_ntop */
#include <stdlib.h>     /* malloc */
#include <stdio.h>      /* I/O */
#include <string.h>     /* string utilties */
#include <stdarg.h>     /* va_list */
#include <assert.h>     /* assert */

#include "IdDigits.h" /* ID_DIGITS */

const uint16_t ADMISSION_PORT_NUMBER = 3300 + ID_DIGITS;


void atomicPrintf(const char *format, ...)
{
    flockfile(stdout);

    va_list args;
    va_start(args, format);
    int vfprintfSucceed = (vfprintf(stdout, format, args) > 0);
    va_end(args);

    int fflushSuceeded = (fflush(stdout) == 0);

    funlockfile(stdout);

    /* assert successes after stdout has been unlocked */
    assert(vfprintfSucceed);
    assert(fflushSuceeded);
}

int createSocket(int type)
{
    int sockFd = socket(AF_INET, type, 0);
    assert(sockFd != -1);
    return sockFd;
}

void getAddress(int                 sockFd,
                struct sockaddr_in *address)
{
    socklen_t addressLength = sizeof(*address);
    (void) memset(address, 0, addressLength);
    assert(getsockname(sockFd,
                       (struct sockaddr *) address,
                       &addressLength) == 0);
    assert(addressLength == sizeof(*address));
}

uint32_t getIp(int client)
{
    struct sockaddr_in address;
    getAddress(client, &address);
    return address.sin_addr.s_addr;
}

void announceSocket(const char *name,
                    const char *trailer,
                    int         sockFd)
{
    /* get the connection type */
    int       sockType     = 0;
    socklen_t sizeSockType = sizeof(sockType);
    assert(getsockopt(sockFd,
                      SOL_SOCKET, /* option is at sockets API level */
                      SO_TYPE,
                      &sockType,
                      &sizeSockType) == 0);
    assert(sizeSockType == sizeof(sockType));
    const char *connectionType = (sockType == SOCK_STREAM) ? "TCP" : "UDP";

    /* get the port number and IP address */
	struct sockaddr_in address;
    getAddress(sockFd, &address);

    uint16_t port = ntohs(address.sin_port); /* network->host byte order */

    /* translate the IP address to something printable */
    char ipAddress[INET_ADDRSTRLEN + 1] = "";
	assert(inet_ntop(AF_INET,
                     &address.sin_addr,
                     ipAddress,
                     sizeof(ipAddress)));

    atomicPrintf(
      "%s has %s port %u and IP address %s%s\n",
      name,
      connectionType,
      (unsigned int) port,
      ipAddress,
      trailer
    );
}

char *packShort(char     *buffer,
                uint16_t  integer)
{
    uint16_t *shortBuffer = (uint16_t *) buffer;
    integer = htons(integer); /* switch to network byte order */
    *shortBuffer++ = integer; /* set and advance buffer */
    return (char *) shortBuffer;
}

char *packString(char       *buffer,
                 const char *string,
                 uint16_t    length)
{
    buffer = packShort(buffer, length);    /* pack the length first */
    (void) memcpy(buffer, string, length); /* then pack the actual string */
    return buffer + length;                /* advance the buffer */
}

int receiveShort(int       sockFd,
                 uint16_t *integer)
{
    DEBUG_LOG("enter %d", sockFd);
    uint16_t recvInteger = 0;
    if (recv(sockFd,
             &recvInteger,
             sizeof(recvInteger),
             0) != sizeof(recvInteger)) {
        return 0;
    }

    *integer = ntohs(recvInteger);
    DEBUG_LOG("%d received: %u", sockFd, (unsigned int)*integer);
    return 1;
}

int receiveString(int    sockFd,
                  char **string)
{
    DEBUG_LOG("enter %d", sockFd);
    /* recv() the length */
    uint16_t length = 0;
    if (!receiveShort(sockFd,
                      &length)) {
        return 0;
    }

    /* allocate room for the string + '\0' */
    char *recvString = malloc(length + 1);
    if (!recvString) {
        return 0;
    }

    /* recv() the actual string */
    if (recv(sockFd,
             recvString,
             length,
             0) != length) {
        free(recvString);
        return 0;
    }

    recvString[length] = '\0'; /* terminate string */
    *string = recvString;

    DEBUG_STRING(recvString, length, "%d received", sockFd);

    return 1;
}
