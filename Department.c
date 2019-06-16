#include <sys/types.h>  /* fork */
#include <sys/socket.h> /* socket */
#include <sys/wait.h>   /* wait */
#include <arpa/inet.h>  /* htons */
#include <unistd.h>     /* fork */
#include <stdio.h>      /* I/O */
#include <stdlib.h>     /* malloc/free */
#include <string.h>     /* string utilities */
#include <assert.h>     /* assert */

#include "AdmissionInterface.h"           /* ADMISSION_PORT_NUMBER */
#include "AdmissionDepartmentInterface.h" /* DEPARTMENTS */
#include "AdmissionClient.h"              /* client utilties */


static FILE *openInputFile(char letter)
{
    char inputPathname[] = "department .txt";
    char *letterPtr      = &inputPathname[sizeof("department") - 1];
    *letterPtr = letter;
    FILE *input = fopen(inputPathname, "r");
    assert(input);
    return input;
}

static int connectToAdmission()
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

    return admission;
}

static size_t packDepartmentInfo(uint16_t     id,
                                 const char  *program,
                                 const char  *minGpa,
                                 char       **buffer)
{
    uint16_t lengthDepartment = (uint16_t) strlen(program);
    uint16_t lengthMinGpa     = (uint16_t) strlen(minGpa);

    size_t bufferSize = sizeof(id)
                      + sizeof(lengthDepartment) + lengthDepartment
                      + sizeof(lengthMinGpa)     + lengthMinGpa;

    char *bufferCursor = malloc(bufferSize);
    assert(bufferCursor && "malloc() failure");
    *buffer = bufferCursor;

    bufferCursor = packShort(bufferCursor, id);
    bufferCursor = packString(bufferCursor, program, lengthDepartment);
    (void)         packString(bufferCursor, minGpa,     lengthMinGpa);

    return bufferSize;
}

static void sendDepartmentInfo(int         admission,
                               uint16_t    id,
                               const char *program,
                               const char *minGpa)
{
    char *buffer = NULL;
    size_t bufferSize = packDepartmentInfo(id,
                                           program,
                                           minGpa,
                                           &buffer);
    assert(send(admission,
                buffer,
                bufferSize,
                0) == bufferSize);
    free(buffer);
}

static void departmentPhase1(uint16_t id)
{
    const struct Department *dep = &DEPARTMENTS[id];

    FILE *input   = openInputFile(dep->letter);
    int admission = connectToAdmission();

    char *program = NULL;
    char *minGpa  = NULL;
    while (readConfig(input, '#', &program, &minGpa)) {
        sendDepartmentInfo(admission, id, program, minGpa);
        free(program);
        free(minGpa);
    }

    assert(close(admission) == 0);
    assert(fclose(input) == 0);

    exit(EXIT_SUCCESS);
}


int main()
{
    uint16_t id = 0;
    for (; id < COUNT_DEPARTMENTS; ++id) {
        pid_t forkStatus = fork();
        if (forkStatus == 0) {
            departmentPhase1(id);
        }
        assert(forkStatus > 0);
    }

    /* wait for child processes */
    int exitStatus  = EXIT_SUCCESS;
    int childStatus = EXIT_SUCCESS;
    while (wait(&childStatus) < 0) {
        exitStatus |= childStatus;
    }

    return exitStatus;
}
