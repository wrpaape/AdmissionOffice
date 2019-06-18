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


/**
 * @brief open a Department's input file in the current directory named
 *     "department<letter>.txt"
 * @param[in] letter the department's letter
 * @return the input file open for reading
 */
static FILE *openInputFile(char letter)
{
    char inputPathname[] = "department#.txt";
    char *letterPtr      = &inputPathname[sizeof("department") - 1];
    *letterPtr = letter;
    FILE *input = fopen(inputPathname, "r");
    assert(input);
    return input;
}

/**
 * @brief allocate a buffer, then copy the program info into the message format
 *     to be delivered to the Admission server
 * @param[in]  id      the ID used to uniquely identify a Department (see
 *     DepartmentRegistrar.h)
 * @param[in]  program the program name
 * @param[in]  minGpa  the program's minimum acceptable GPA
 * @param[out] buffer the program info message
 * @return the byte size of the @p buffer
 */
static size_t packDepartmentInfo(uint16_t     id,
                                 const char  *program,
                                 const char  *minGpa,
                                 char       **buffer)
{
    uint16_t lengthDepartment = (uint16_t) strlen(program);
    uint16_t lengthMinGpa     = (uint16_t) strlen(minGpa);

    /* allocate a buffer big enough */
    size_t bufferSize = sizeof(id)
                      + sizeof(lengthDepartment) + lengthDepartment
                      + sizeof(lengthMinGpa)     + lengthMinGpa;

    char *bufferCursor = malloc(bufferSize);
    assert(bufferCursor && "malloc() failure");
    *buffer = bufferCursor;

    /* pack the program info message */
    bufferCursor = packShort(bufferCursor, id);                         /* department ID */
    bufferCursor = packString(bufferCursor, program, lengthDepartment); /* program name */
    (void)         packString(bufferCursor, minGpa,  lengthMinGpa);     /* minimum GPA  */

    return bufferSize;
}

/**
 * @brief send the configured program info in a packet
 * @param[in] admission 
 * @param[in] id        
 * @param[in] program
 * @param[in] minGpa
 */
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

/**
 * @brief The phase 1 routine for a Department instance.
 * @param The phase 1 routine for a Department instance.
 */
static void departmentPhase1(uint16_t id)
{
    const struct Department *dep = &DEPARTMENTS[id - 1];

    FILE *input = openInputFile(dep->letter);
    char name[] = "<Department#>";
    name[sizeof(name) - 3] = dep->letter; /* index of # */

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
            "%s has sent <%s> to the admission office\n",
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
