#include "AdmissionClient.h"

#include <stdlib.h>     /* free */
#include <string.h>     /* string utilities */
#include <arpa/inet.h>  /* htons */
#include <assert.h>     /* assert */

#include "AdmissionCommon.h"


const char *ADMISSION_IP_ADDRESS = "127.0.0.1"; /* localhost */

int connectToAdmission(const char *client,
                       const char *trailer)
{
    int admission = socket(AF_INET, SOCK_STREAM, 0);
    assert(admission != -1);

	struct sockaddr_in admissionAddress;
    (void) memset(&admissionAddress, 0, sizeof(admissionAddress));
	admissionAddress.sin_family = AF_INET;
	admissionAddress.sin_port   = htons(ADMISSION_PORT_NUMBER);
	assert(inet_pton(AF_INET,
                     ADMISSION_IP_ADDRESS,
                     &admissionAddress.sin_addr) == 1);

	assert(connect(admission,
                   (const struct sockaddr *) &admissionAddress,
                   sizeof(admissionAddress)) == 0);

    announceConnection(client, trailer, admission);

    return admission;
}

int readConfig(FILE  *input,
               char   delimiter,
               char **key,
               char **value)
{
    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), input)) {
        return 0;
    }
    char *split = strchr(buffer, delimiter);
    if (!split) {
        return 0;
    }
    *split = '\0'; /* split */
    char *keyCopy = strdup(buffer);
    if (!keyCopy) {
        return 0;
    }
    char *valueBegin = split + 1;
    char *valueEnd   = valueBegin;
    while (   (*valueEnd != '\r')
           && (*valueEnd != '\n')
           && (*valueEnd != '\0')) {
        ++valueEnd;
    }
    *valueEnd = '\0'; /* remove trailing newline */
    char *valueCopy = strdup(valueBegin);
    if (!valueCopy) {
        free(keyCopy);
        return 0;
    }
    *key   = keyCopy;
    *value = valueCopy;
    return 1;
}

char *packShort(char     *buffer,
                uint16_t  integer)
{
    uint16_t *shortBuffer = (uint16_t *) buffer;
    integer = htons(integer); /* switch to network byte order */
    *shortBuffer++ = integer;
    return (char *) shortBuffer;
}

char *packString(char       *buffer,
                 const char *string,
                 uint16_t    length)
{
    buffer = packShort(buffer, length);
    (void) memcpy(buffer, string, length);
    return buffer + length;
}
