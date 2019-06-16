#include "AdmissionCommon.h"

#include <sys/socket.h> /* getsockname */
#include <arpa/inet.h>  /* inet_ntop */
#include <string.h>     /* string utilties */
#include <stdio.h>      /* I/O */
#include <assert.h>     /* assert */

#include "IdDigits.h" /* ID_DIGITS */


const uint16_t ADMISSION_PORT_NUMBER = 3300 + ID_DIGITS;

void announceConnection(const char *name,
                        const char *trailer,
                        int         socket)
{
	struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);
    (void) memset(&address, 0, addressLength);
    assert(getsockname(socket,
                       (struct sockaddr *) &address,
                       &addressLength) == 0);
    assert(addressLength == sizeof(address));

    uint16_t port = ntohs(address.sin_port);

    char ipAddress[INET_ADDRSTRLEN + 1] = "";
	assert(inet_ntop(AF_INET,
                     &address.sin_addr,
                     ipAddress,
                     sizeof(ipAddress)));

    assert(printf(
      "%s has TCP port %u and IP address %s%s\n",
      name,
      (unsigned int) port,
      ipAddress,
      trailer
    ) >= 0);
}
