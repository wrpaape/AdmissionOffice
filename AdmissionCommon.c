#include "AdmissionCommon.h"

#include <sys/socket.h> /* getsockname */
#include <arpa/inet.h>  /* inet_ntop */
#include <string.h>     /* string utilties */
#include <stdio.h>      /* I/O */
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

void announceConnection(const char *name,
                        const char *trailer,
                        int         socket)
{
    /* get the port number and IP address */
	struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);
    (void) memset(&address, 0, addressLength);
    assert(getsockname(socket,
                       (struct sockaddr *) &address,
                       &addressLength) == 0);
    assert(addressLength == sizeof(address));

    uint16_t port = ntohs(address.sin_port); /* network->host byte order */

    /* translate the IP address to something printable */
    char ipAddress[INET_ADDRSTRLEN + 1] = "";
	assert(inet_ntop(AF_INET,
                     &address.sin_addr,
                     ipAddress,
                     sizeof(ipAddress)));

    atomicPrintf(
      "%s has TCP port %u and IP address %s%s\n",
      name,
      (unsigned int) port,
      ipAddress,
      trailer
    );
}
