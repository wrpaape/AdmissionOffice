#include <sys/types.h>  /* fork */
#include <sys/socket.h> /* socket */
#include <sys/wait.h>   /* wait */
#include <arpa/inet.h>  /* htons */
#include <unistd.h>     /* fork */
#include <stdio.h>      /* I/O */
#include <stdlib.h>     /* malloc/free */
#include <string.h>     /* string utilities */
#include <assert.h>     /* assert */

#include "AdmissionCommon.h"     /* ADMISSION_PORT_NUMBER */
#include "DepartmentRegistrar.h" /* DEPARTMENTS */
#include "AdmissionClient.h"     /* client utilties */


static FILE *openInputFile(char letter)
{
    char inputPathname[] = "department .txt";
    char *letterPtr      = &inputPathname[sizeof("department") - 1];
    *letterPtr = letter;
    FILE *input = fopen(inputPathname, "r");
    assert(input);
    return input;
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
    const struct Department *dep = &DEPARTMENTS[id - 1];

    FILE *input = openInputFile(dep->letter);
    char name[] = "Department_";
    name[sizeof(name) - 2] = dep->letter;

    int admission = connectToAdmission(name, " for Phase 1");
    assert(printf(
        "%s is now connected to the admission office\n",
        name
    ) >= 0);

    char *program = NULL;
    char *minGpa  = NULL;
    while (readConfig(input, '#', &program, &minGpa)) {
        sendDepartmentInfo(admission, id, program, minGpa);
        assert(printf(
            "%s has sent %s to the admission office\n",
            name,
            program
        ) >= 0);
        free(program);
        free(minGpa);
    }

    assert(close(admission) == 0);
    assert(fclose(input) == 0);

    assert(printf(
        "Updating the admission office is done for %s\n",
        name
    ) >= 0);

    exit(EXIT_SUCCESS);
}


int main()
{
    uint16_t id = 1;
    for (; id <= COUNT_DEPARTMENTS; ++id) {
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
