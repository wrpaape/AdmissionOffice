#include "AdmissionClient.h"

#include <stdlib.h>     /* free() */
#include <stdio.h>      /* snprintf() */
#include <string.h>     /* string utilities */
#include <arpa/inet.h>  /* htons() */
#include <sys/types.h>  /* getaddrinfo() */
#include <sys/socket.h> /* getaddrinfo() */
#include <netdb.h>      /* getaddrinfo() */
#include <pthread.h>    /* pthread_mutex_t */
#include <assert.h>     /* assert() */

#include "AdmissionCommon.h"


static struct addrinfo* getAddressInfo(const char *node,
                                       uint16_t    port,
                                       int         type,
                                       int         flags)
{

    char portString[sizeof("65535")] = ""; /* big enough for largest port */
    assert(snprintf(portString,
                    sizeof(portString),
                    "%u",
                    (unsigned int) port) >= 0);

    struct addrinfo hints;
    (void) memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET; /* force IPv4 */
    hints.ai_socktype = type;    /* TCP */
    hints.ai_flags    = flags;   /* TCP */

    struct addrinfo *info = NULL;
    assert(getaddrinfo(node,
                       portString,
                       NULL,
                       &info) == 0);
    return info;
}

static uint32_t lookupAdmissionIp()
{
    /**
     * The node where the Admission server is running
     */
    static const char *ADMISSION_NODE = "localhost";

    struct addrinfo *admissionInfo = getAddressInfo(ADMISSION_NODE,
                                                    ADMISSION_PORT_NUMBER,
                                                    SOCK_STREAM,
                                                    0);
    assert(admissionInfo && "failed to find admission address info");
    assert(   (admissionInfo->ai_family == AF_INET)
           && "no available IPv4 address");
    const struct sockaddr_in *ipv4Address
        = (const struct sockaddr_in*) admissionInfo->ai_addr;
    assert(ipv4Address && "null IPv4 address");

    uint32_t admissionIp = ipv4Address->sin_addr.s_addr;

    freeaddrinfo(admissionInfo);

    return admissionIp;
}

static void unlockMutex(void *arg)
{
    pthread_mutex_t *lock = (pthread_mutex_t *) arg;
    (void) pthread_mutex_unlock(lock);
}

/**
 * @brief get the Admission Office's IP address
 * @details caches the IP on successive calls
 */
static uint32_t getAdmissionIp()
{
    static pthread_mutex_t ADMISSION_IP_LOCK        = PTHREAD_MUTEX_INITIALIZER;
    static uint32_t        ADMISSION_IP             = 0;
    static int             ADMISSION_IP_INITIALIZED = 0;

    uint32_t admissionIp = 0;

    /* unlock in case getaddrinfo is cancelled */
    pthread_cleanup_push(&unlockMutex, &ADMISSION_IP_LOCK);

    assert(pthread_mutex_lock(&ADMISSION_IP_LOCK) == 0);

    if (!ADMISSION_IP_INITIALIZED) {
        /* have to look up the IP */
        ADMISSION_IP             = lookupAdmissionIp();
        ADMISSION_IP_INITIALIZED = 1;
    }
    admissionIp = ADMISSION_IP;

    pthread_cleanup_pop(1); /* unlock the mutex */

    return admissionIp;
}

int connectToAdmission(const char *client,
                       const char *trailer)
{
    /* IPv4 address and connection-oriented protocol */
    int admission = createSocket(SOCK_STREAM);

    /* convert the string representation of the IP address */
	struct sockaddr_in admissionAddress;
    (void) memset(&admissionAddress, 0, sizeof(admissionAddress));
	admissionAddress.sin_family      = AF_INET;
	admissionAddress.sin_port        = htons(ADMISSION_PORT_NUMBER);
    admissionAddress.sin_addr.s_addr = getAdmissionIp();

    /* connect() */
	assert(connect(admission,
                   (const struct sockaddr *) &admissionAddress,
                   sizeof(admissionAddress)) == 0);

    /* announce TCP port and IP address */
    announceSocket(client, trailer, admission);

    return admission;
}

int openAdmissionListener(uint16_t port)
{
    /* create a socket for receiving messages from the admissions office */
    int listener = createSocket(SOCK_DGRAM);

    /* bind() the socket to the loopback address and the provided port */
    bindToLoopback(listener, port);

    return listener;
} 

void announceAdmissionListener(const char *client,
                               int         listener)
{
    announceSocket(client, " for Phase 2", listener);
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

    /* success */
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
