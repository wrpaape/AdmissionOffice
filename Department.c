#include <sys/types.h>  /* fork */
#include <sys/socket.h> /* send */
#include <unistd.h>     /* fork */
#include <sys/wait.h>   /* wait */
#include <stdlib.h>     /* malloc/free */
#include <string.h>     /* string utilities */
#include <assert.h>     /* assert */

#include "AdmissionCommon.h"     /* ADMISSION_PORT_NUMBER */
#include "AdmissionClient.h"     /* client utilties */
#include "DepartmentRegistrar.h" /* DEPARTMENTS */


/**
 * @brief open a Department's input file in the current directory named
 *     "department<letter>.txt"
 * @param[in] letter the Department's letter
 * @return the input file open for reading
 */
static FILE *openInputFile(char letter)
{
    char inputPathname[] = "department#.txt";
    char *letterPtr      = &inputPathname[sizeof("department") - 1];
    *letterPtr = letter;
    FILE *input = fopen(inputPathname, "r");
    assert(input && "fopen() failure");
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
 * @brief send a packet the configured program info in a packet
 * @param[in] admission the socket connection to the Admission office
 * @param[in] id        the ID of @p program's department
 * @param[in] program   the name of the program
 * @param[in] minGpa    the minimum acceptable GPA for the program
 */
static void sendDepartmentInfo(int         admission,
                               uint16_t    id,
                               const char *program,
                               const char *minGpa)
{
    /* pack the information up into a single buffer */
    char *buffer = NULL;
    size_t bufferSize = packDepartmentInfo(id,
                                           program,
                                           minGpa,
                                           &buffer);
    /* send it */
    assert(send(admission,
                buffer,
                bufferSize,
                0) == ((ssize_t) bufferSize));
    free(buffer);
}

/**
 * @brief the phase 1 routine for a Department instance
 * @param[in] name             the name of this Department instance
 * @param[in] id               the ID of a particular registered Department
 *     (see DepartmentRegistrar.h)
 * @param[in] departmentLetter this registered Department's letter
 */
static void departmentPhase1(const char *name,
                             uint16_t    id,
                             char        departmentLetter)
{
    /* open the config file */
    FILE *input = openInputFile(departmentLetter);

    /* connect to the Admission server */
    int admission = connectToAdmission(name, " for Phase 1");
    atomicPrintf("%s is now connected to the admission office\n", name);

    /* read each line of the config file */
    char *program = NULL;
    char *minGpa  = NULL;
    while (readConfig(input, '#', &program, &minGpa)) {
        /* send each program info line to the Admission office */
        sendDepartmentInfo(admission, id, program, minGpa);
        atomicPrintf(
            "%s has sent <%s> to the admission office\n",
            name,
            program
        );
        free(program);
        free(minGpa);
    }

    /* close the Admission server connection */
    assert(close(admission) == 0);

    /* close the config file */
    assert(fclose(input) == 0);

    atomicPrintf(
        "Updating the admission office is done for %s\n",
        name
    );

    atomicPrintf("End of Phase 1 for %s\n", name);
}

static const char *getStudentName(char *admissionMessage)
{
    /* student name is listed first */
    const char *studentName = admissionMessage;
    
    /* chop message of at first '#' delimiter */
    char *endOfStudent = strchr(admissionMessage, '#');
    assert(endOfStudent && "ill-formed admission message");
    *endOfStudent = '\0'; /* terminate student name */

    return studentName;
}

/**
 * @brief if receive a UDP packet with a leading 0, interpret that as the end
 *     of the stream of Program Admission packets
 */
static int doneListening(int listener)
{
    return (peekShort(listener) == 0);
}

/**
 * @brief the phase 2 routine for a Department instance
 * @param[in] name     the name of this Department instance
 * @param[in] listener the UDP socket to receive admission messages
 */
static void departmentPhase2(const char *name,
                             int         listener)
  
{
    DEBUG_LOG("%s enter phase 2", name);
    announceAdmissionListener(name, listener);

    char *admissionMessage = NULL;
    /* until the Admission Office tells us it's done processing applications...
     */
    while (!doneListening(listener)) {

        /* accept a new program ladmission notification of the form "<Student
         * Name>#<Student GPA>#<Admitted Program>" */
        assert(   listenForString(listener, &admissionMessage)
               && "expected an admission message");

        DEBUG_LOG("%s - received admission message: \"%s\"\n",
                  name,
                  admissionMessage);

        const char *studentName = getStudentName(admissionMessage);

        atomicPrintf("<%s> has been admitted to %s\n", studentName, name);

        free(admissionMessage);
    }

    atomicPrintf("End of Phase 2 for %s\n", name);
}

/**
 * @brief The routine for a Department instance.
 * @param[in] id the ID of a particular registered Department (see
 *     DepartmentRegistrar.h)
 */
static void department(uint16_t id)
{
    /* fetch the Department info */
    const struct Department *dep = &DEPARTMENTS[id - 1];

    /* build the name (for output messages) */
    char name[] = "<Department#>";
    name[sizeof(name) - 3] = dep->letter; /* index of # */

    /* open the UDP socket at start to prevent race conditions of server
     * sending before ready */
    int listener = openAdmissionListener(dep->port);

    /* phase 1 */
    departmentPhase1(name, id, dep->letter);

    /* phase 2 */
    departmentPhase2(name, listener);

    /* close() the UDP socket */
    assert(close(listener) == 0);
}

int main()
{
    /* spawn a child process for all registered Department instances
     * (see DepartmentRegistrar.h) */
    uint16_t id = 1;
    for (; id <= COUNT_DEPARTMENTS; ++id) {
        pid_t forkStatus = fork();
        if (forkStatus == 0) {
            /* child process (Department instance) */
            department(id);
            exit(EXIT_SUCCESS);
        }
        assert((forkStatus > 0) && "fork() failure");
    }

    /* wait for child processes */
    int exitStatus  = EXIT_SUCCESS;
    int childStatus = EXIT_SUCCESS;
    while (wait(&childStatus) >= 0) {
        /* if any child processes fail (nonzero), exit with a nonzero status*/
        exitStatus |= childStatus;
    }

    return exitStatus;
}
