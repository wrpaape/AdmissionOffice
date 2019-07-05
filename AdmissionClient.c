#include "AdmissionClient.h"

#include <stdlib.h>     /* free() */
#include <stdio.h>      /* snprintf() */
#include <string.h>     /* string utilities */
#include <arpa/inet.h>  /* htons() */
#include <sys/types.h>  /* getaddrinfo() */
#include <sys/socket.h> /* getaddrinfo() */
#include <netdb.h>      /* getaddrinfo() */
#include <assert.h>     /* assert() */

#include "AdmissionCommon.h"


/**
 * The IP address of the Admission server
 */
const char *ADMISSION_IP_ADDRESS = "127.0.0.1"; /* localhost */


int connectToAdmission(const char *client,
                       const char *trailer)
{
    /* IPv4 address and connection-oriented protocol */
    int admission = createSocket(SOCK_STREAM);

    /* convert the string representation of the IP address */
	struct sockaddr_in admissionAddress;
    (void) memset(&admissionAddress, 0, sizeof(admissionAddress));
	admissionAddress.sin_family = AF_INET;
	admissionAddress.sin_port   = htons(ADMISSION_PORT_NUMBER);
	assert(inet_pton(AF_INET,
                     ADMISSION_IP_ADDRESS,
                     &admissionAddress.sin_addr) == 1);

    /* connect() */
	assert(connect(admission,
                   (const struct sockaddr *) &admissionAddress,
                   sizeof(admissionAddress)) == 0);

    /* announce TCP port and IP address */
    announceSocket(client, trailer, admission);

    return admission;
}

static void bindListener(int      listener,
                         uint16_t port)
{
    char portString[sizeof("65535")] = ""; /* big enough for largest port */
    assert(snprintf(portString,
                    sizeof(portString),
                    "%u",
                    (unsigned int) port) >= 0);

    struct addrinfo hints;
    (void) memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;    /* force IPv4 */
    hints.ai_socktype = SOCK_DGRAM; /* UDP */
    hints.ai_flags    = AI_PASSIVE; /* use my IP */

    struct addrinfo *localInfo = NULL;
    assert(getaddrinfo(NULL,
                       portString,
                       &hints,
                       &localInfo) == 0);

    struct addrinfo *info = localInfo;
    for (; info != NULL; info = info->ai_next) {
        /* attempt to bind() the socket */
        if (bind(listener,
                 info->ai_addr,
                 info->ai_addrlen) == 0) {
            freeaddrinfo(localInfo);
            return;
        }
    }

    assert(!"bindListener() failure");
}

int openAdmissionListener(uint16_t    port,
                          const char *client)
{
    /* create a socket for receiving messages from the admissions office */
    int listener = createSocket(SOCK_DGRAM);

    /* bind() the socket */
    bindListener(listener, port);
	/* struct sockaddr_in address; */
    /* (void) memset(&address, 0, sizeof(address)); */
	/* address.sin_family      = AF_INET; */
	/* address.sin_addr.s_addr = getIp(listener); */
	/* address.sin_port        = htons(port); */
    /* assert(bind(listener, */
                /* (const struct sockaddr *) &address, */
                /* sizeof(address)) == 0); */

    announceSocket(client, " for Phase 2", listener);

    return listener;
} 

uint16_t peekShort(int sockFd)
{
    DEBUG_LOG("enter %d", sockFd);
    uint16_t peekedShort = 0;
    if (recv(sockFd,
             &peekedShort,
             sizeof(peekedShort),
             MSG_PEEK) != sizeof(peekedShort)) {
        return 0;
    }
    peekedShort = ntohs(peekedShort); /* correct the byte order */
    DEBUG_LOG("%d peeked %u", sockFd, (unsigned int) peekedShort);
    return peekedShort;
}

int listenForString(int    listener,
                    char **string)
{
    DEBUG_LOG("enter %d", listener);

    /* peek (MSG_PEEK) at the length */
    uint16_t length = peekShort(listener);

    /* allocate room for <length> + the string */
    size_t sizeBuffer = sizeof(length) + length;
    char *recvString = malloc(sizeBuffer);
    if (!recvString) {
        return 0;
    }

    /* recv() the <length> + string */
    if (recv(listener,
             recvString,
             sizeBuffer,
             0) != (ssize_t) sizeBuffer) {
        free(recvString);
        return 0;
    }

    /* overwrite the <length> header */
    (void) memmove(recvString,
                   recvString + sizeof(length),
                   length);

    recvString[length] = '\0'; /* terminate string */
    *string = recvString;

    DEBUG_STRING(recvString, length, "%d received", listener);

    return 1;
}

int readConfig(FILE  *input,
               char   delimiter,
               char ** key,
               char ** value)
{
    /* read a line */
    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), input)) {
        return 0;
    }
    char *split = strchr(buffer, delimiter);
    if (!split) {
        return 0;
    }
    *split = '\0'; /* split at the delimiter */
    char *const keyCopy = strdup(buffer); /* copy the key */
    if (!keyCopy) {
        return 0;
    }

    char *valueBegin = split + 1;

    /* remove control characters '\r' or '\n', if they are present at the end
     * of the line */
    char *valueEnd = valueBegin;
    while (   (*valueEnd != '\r')
           && (*valueEnd != '\n')
           && (*valueEnd != '\0')) {
        ++valueEnd;
    }
    *valueEnd = '\0';

    char *const valueCopy = strdup(valueBegin); /* copy the value */
    if (!valueCopy) {
        free(keyCopy);
        return 0;
    }
    *key   = keyCopy;
    *value = valueCopy;
    return 1; /* success */
}
