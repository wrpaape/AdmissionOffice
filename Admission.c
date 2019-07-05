#include <sys/types.h>  /* socket */
#include <sys/socket.h> /* socket */
#include <unistd.h>     /* close */
#include <arpa/inet.h>  /* htons, htonl */
#include <stdlib.h>     /* strtod */
#include <stdio.h>      /* I/O */
#include <string.h>     /* memset */
#include <pthread.h>    /* pthread */
#include <assert.h>     /* assert */

#include "AdmissionCommon.h"     /* ADMISSION_PORT_NUMBER */
#include "DepartmentRegistrar.h" /* DEPARTMENTS */
#include "AdmissionDb.h"         /* AdmissionDb */
#include "StudentRegistrar.h"    /* COUNT_STUDENTS */


/**
 * A collection of arguments and data structures needed to handle a Department
 * connection in Phase 1.
 */
struct DepartmentHandler {
    int              department;    /** the department connection socket */
    AdmissionDb     *aDb;           /** the accumulating DB of program info */
    pthread_mutex_t *aDbLock;       /** the lock to synchronize access to aDb */
    uint32_t        *departmentIps; /** the array of department IP addresses */
    pthread_t        thread;        /** the thread handle */
};

/**
 * A collection of arguments and data structures needed to handle a Student
 * connection in Phase 2.
 */
struct StudentHandler {
    int                student;       /** the student connection socket */
    const AdmissionDb *aDb;           /** the complete DB of program info */
    const uint32_t    *departmentIps; /** the array of department IP addresses */
    pthread_t          thread;        /** the thread handle */
};

/**
 * NOTE: this code is mostly reused from my Lab 2 assignment's server socket
 * setup routine create_server_socket()
 * @brief create a TCP "server" socket
 * @return the Admission server socket
 */
static int createAdmissionSocket()
{
    static const int ADMISSION_BACKLOG = 128;

    /* open a TCP socket with a IPv4 address */
    int admission = createSocket(SOCK_STREAM);

    static const int option = SO_REUSEADDR  /* allow other sockets to bind() to this port */
                            | SO_REUSEPORT; /* force reuse of this port */
    int optionValue = 1; /* true */
    assert(setsockopt(admission,
                      SOL_SOCKET, /* manipulate options at the sockets API level */
                      option,
                      &optionValue,
                      sizeof(optionValue)) == 0);

    /* bind() the socket */
	struct sockaddr_in address;
    (void) memset(&address, 0, sizeof(address));
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port        = htons(ADMISSION_PORT_NUMBER);
    assert(bind(admission,
                (const struct sockaddr *) &address,
                sizeof(address)) == 0);

    /* listen() */
    assert(listen(admission, ADMISSION_BACKLOG) == 0);
    
    return admission;
}

/**
 * @brief recv() a floating point value from the @p sockFd
 * @param[in]  sockFd the socket
 * @param[out] value the result
 * @return non-zero if successful
 */
static int receiveDouble(int     sockFd,
                         double *value)
{
    DEBUG_LOG("enter %d", sockFd);

    /* doubles are communicated as an ASCII string over the network - read in
     * the raw string */
    char *valueString = NULL;
    if (!receiveString(sockFd,
                       &valueString)) {
        return 0;
    }

    /* attempt to parse the double value from it */
    char *conversionEnd = NULL;
    *value = strtod(valueString, &conversionEnd);

    int conversionSucceeded = (conversionEnd != valueString);
    free(valueString);

    DEBUG_LOG("%d received %f", sockFd, *value);

    return conversionSucceeded;
}

/**
 * @brief recv() the programming info message
 * @param[in]  department the socket connection with a department
 * @param[out] program    the program name
 * @param[out] minGpa     the program's minimum acceptable GPA
 * @return non-zero if successful
 */
static int receiveProgramInfo(int       department,
                              char    **program,
                              double   *minGpa,
                              uint16_t *departmentId)
{
    char *recvProgram         = NULL;
    uint16_t recvDepartmentId = 0;
    int success = receiveShort(department, &recvDepartmentId) /* recv() ID */
               && (recvDepartmentId >= 1)                     /*   validate ID */
               && (recvDepartmentId <= COUNT_DEPARTMENTS)     /*   validate ID */
               && receiveString(department, &recvProgram)     /* recv() programe name */
               && receiveDouble(department, minGpa);          /* recv() minimum acceptable GPA */

    if (success) {
        *program      = recvProgram;
        *departmentId = recvDepartmentId;
    } else {
        free(recvProgram);
    }

    return success;
}

/**
 * @brief accept() a TCP connection from a client
 * @param[in] admission the listen()ing admission office socket
 * @return the socket connection with the client
 */
static int acceptClient(int admission)
{
    struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);
    (void) memset(&address, 0, addressLength);
    int client = accept(admission,
                        (struct sockaddr *) &address,
                        &addressLength);
    assert(client != -1);

    DEBUG_LOG("admission office accepted TCP connection from port %u",
              ntohs(address.sin_port));

    return client;
}

/**
 * @brief recv() all program info messages from a @p department and store it in
 *     the @p aDb, then close() the connection
 * @param[in] department the socket connection to the department
 * @param[in] aDb        the accumulating database of program info
 * @param[in] aDbLock    the mutex protecting writes to @p aDb
 */
static void handleDepartment(int              department,
                             AdmissionDb     *aDb,
                             pthread_mutex_t *aDbLock,
                             uint32_t        *departmentIps)
{
    char     *program          = NULL;
    double    minGpa           = 0.0;
    uint16_t  departmentId     = 0;
    uint16_t  prevDepartmentId = 0;
    while (receiveProgramInfo(department,
                              &program,
                              &minGpa,
                              &departmentId)) {
        if (prevDepartmentId != 0) {
            /* check that each department message starts with the same ID */
            assert((departmentId == prevDepartmentId)
                   && "Received inconsistent department IDs");
        }

        /* insert the program info into the DB */
        assert(pthread_mutex_lock(aDbLock) == 0);
        int successfulAdd = aDbAdd(aDb, program, minGpa, departmentId);
        assert(pthread_mutex_unlock(aDbLock) == 0);

        /* ensure the insertion succeeded after lock relinquished */
        assert(successfulAdd);

        free(program);
        prevDepartmentId = departmentId;
    }

    /* check that at least 1 program info message was sent */
    if (departmentId != 0) {
        char departmentLetter = DEPARTMENTS[departmentId - 1].letter;
        atomicPrintf("Received the program list from <Department%c>\n",
                     departmentLetter);
        /* save the IP for this department */
        departmentIps[departmentId - 1] = getIp(department);
    }

    /* close() the connection */
    assert(close(department) == 0);
}

static void *runHandleDepartment(void *arg)
{
    const struct DepartmentHandler *handler
        = (const struct DepartmentHandler *) arg;

    /* recv() their program info and store it in the DB */
    handleDepartment(handler->department,
                     handler->aDb,
                     handler->aDbLock,
                     handler->departmentIps);
    
    return NULL;
}

/**
 * @brief builds an AdmissionDb of program info received by the registered
 *     departments
 * @return the DB of program info, keyed by program name
 */
static AdmissionDb *admissionPhase1(int       admission,
                                    uint32_t *departmentIps)
{
    /* create an empty DB */
    AdmissionDb *aDb = aDbCreate();
    assert(aDb);

    /* create a lock for the DB */
    pthread_mutex_t aDbLock = PTHREAD_MUTEX_INITIALIZER;

    /* allocate a thread and memory to hold arguments for all COUNT_DEPARTMENTS
     * expected Department clients */
    struct DepartmentHandler *handlers = malloc(
        COUNT_DEPARTMENTS * sizeof(struct DepartmentHandler)
    );
    assert(handlers && "malloc() failure");

    /* for the expected number of registered departments... */
    size_t i = 0;
    for (; i < COUNT_DEPARTMENTS; ++i) {
        /* accept a connection to the next department */
        int department = acceptClient(admission);

        /* set the handler arguments */
        struct DepartmentHandler *handler = &handlers[i];
        handler->department    = department;
        handler->aDb           = aDb;
        handler->aDbLock       = &aDbLock;
        handler->departmentIps = departmentIps;

        /* handle the Department in a separate thread
         * s.t. clients can be handled concurrently */
        assert(pthread_create(&handler->thread,
                              NULL, /* no pthread attributes */
                              &runHandleDepartment,
                              handler) == 0);
    }

    /* join the threads */
    for (i = 0; i < COUNT_DEPARTMENTS; ++i) {
        assert(pthread_join(handlers[i].thread,
                            NULL /* discard retval */) == 0);
    }

    /* free the handlers once all Departments have been served */
    free(handlers);

    /* ready the DB for lookup */
    aDbFinalize(aDb);

    atomicPrintf("End of Phase 1 for the admission office\n");

    return aDb;
}

void receiveInterest(int        student,
                     uint16_t   studentId,
                     char     **program)
{
    DEBUG_LOG("receiving interest from %d", student);

    uint16_t recvStudentId = 0;
    assert(receiveShort(student, &recvStudentId) && "missing student ID");
    assert((recvStudentId == studentId) && "received inconsistent student IDs");
    assert(receiveString(student, program) && "missing program of interest");
}


static int receiveInitialStudentPacket(int       student,
                                       uint16_t *studentId,
                                       double   *studentGpa,
                                       uint16_t *countInterests)
{
    DEBUG_LOG("receiving student ID and GPA...");
    return receiveShort( student, studentId)
        && receiveDouble(student, studentGpa)
        && receiveShort( student, countInterests);
}

static uint16_t processApplication(int                 student,
                                   uint16_t            studentId,
                                   double              studentGpa,
                                   uint16_t            countInterests,
                                   const AdmissionDb  *aDb,
                                   char              **admittedProgram,
                                   uint16_t           *admittedDepartmentId)
{
    char     *firstAdmittedProgram      = NULL;
    uint16_t  firstAdmittedDepartmentId = 0;
    uint16_t  countValidPrograms        = 0;
    char     *program                   = NULL;
    for (; countInterests > 0; --countInterests) {
        receiveInterest(student, studentId, &program);
        double   minGpa       = 0.0;
        uint16_t departmentId = 0;
        int foundProgram = aDbFind(aDb,
                                   program,
                                   &minGpa,
                                   &departmentId);
        countValidPrograms += foundProgram;

        if (   !firstAdmittedProgram
            && foundProgram
            && (studentGpa >= minGpa)) {
            firstAdmittedProgram      = program;
            firstAdmittedDepartmentId = departmentId;
        } else {
            free(program);
        }
    }
    *admittedProgram      = firstAdmittedProgram;
    *admittedDepartmentId = firstAdmittedDepartmentId;
    return countValidPrograms;
}


static void sendApplicationReply(int      student,
                                 uint16_t countValidPrograms)
{
    DEBUG_LOG("Sending application reply (%u)",
              (unsigned int) countValidPrograms);
    char buffer[sizeof(countValidPrograms)];
    (void) packShort(buffer, countValidPrograms);
    assert(send(student,
                buffer,
                sizeof(buffer),
                0) == sizeof(buffer));
}

static size_t makePhase2Packet(const char  *message,
                               char       **packet)
{
    uint16_t lengthMessage = (uint16_t) strlen(message);
    size_t   sizePacket    = sizeof(lengthMessage)
                           + lengthMessage;

    char *buffer = malloc(sizePacket);
    assert(buffer && "malloc() failure");
    (void) packString(buffer, message, lengthMessage);

    DEBUG_STRING(buffer, sizePacket, "examining packet");

    *packet = buffer;
    return sizePacket;
}

static void sendPhase2Message(int         phase2Socket,
                              uint32_t    destinationIp,
                              uint16_t    destinationPort,
                              const char *message,
                              const char *messageDescription,
                              const char *destinationName)
{
	struct sockaddr_in destinationAddress;
    (void) memset(&destinationAddress, 0, sizeof(destinationAddress));
	destinationAddress.sin_family      = AF_INET;
	destinationAddress.sin_port        = htons(destinationPort);
	destinationAddress.sin_addr.s_addr = destinationIp;

    char *packet = NULL;
    size_t sizePacket = makePhase2Packet(message, &packet);

    DEBUG_STRING(packet, sizePacket, "phase 2 message");


    assert(sendto(phase2Socket,
                  packet,
                  sizePacket,
                  0,
                  (const struct sockaddr *) &destinationAddress,
                  sizeof(destinationAddress)) == (ssize_t) sizePacket);

    free(packet);

    atomicPrintf("The admission office has send %s to %s\n",
                 messageDescription,
                 destinationName);
}

static void sendStudentResult(int         phase2Socket,
                              uint32_t    studentIp,
                              uint16_t    studentId,
                              const char *result)
{
    /* retrieve the student port */
    uint16_t studentPort = STUDENT_PORTS[studentId - 1];

    char studentName[(sizeof("<Student65535>"))]; /* max required size */
    assert(snprintf(studentName,
                    sizeof(studentName),
                    "<Student%u>",
                    (unsigned int) studentId) >= 0);

    /* send the result */
    sendPhase2Message(phase2Socket,
                      studentIp,
                      studentPort,
                      result,
                      "the application result",
                      studentName);
}

static void sendStudentAccepted(int         phase2Socket,
                                uint32_t    studentIp,
                                uint16_t    studentId,
                                const char *admittedProgram,
                                char        departmentLetter)
{
    static const char *ACCEPTED_FORMAT = "Accept#%s#department%c";

    /* allocate the message */
    int lengthResult = snprintf(NULL,
                                0,
                                ACCEPTED_FORMAT,
                                admittedProgram,
                                departmentLetter);
    assert((lengthResult >= 0) && "snprintf() failure");

    size_t sizeResult = lengthResult + 1; /* add 1 for '\0' terminator */

    char *message = malloc(sizeResult);
    assert(message && "malloc() failure");

    /* build the message */
    assert(snprintf(message,
                    sizeResult,
                    ACCEPTED_FORMAT,
                    admittedProgram,
                    departmentLetter) == lengthResult);

    /* send the message */
    sendStudentResult(phase2Socket,
                      studentIp,
                      studentId,
                      message);

    /* release the message */
    free(message);
}

static void sendStudentRejected(int      phase2Socket,
                                uint32_t studentIp,
                                uint16_t studentId)
{
    sendStudentResult(phase2Socket, studentIp, studentId, "Rejected");
}

static void sendDepartmentAdmission(int                      phase2Socket,
                                    uint32_t                 departmentIp,
                                    const struct Department *dep,
                                    uint16_t                 studentId,
                                    double                   studentGpa,
                                    const char              *admittedProgram)
{
    static const char *ADMITTED_FORMAT = "Student%u#%f#%s";

    /* allocate the message */
    int lengthResult = snprintf(NULL,
                                0,
                                ADMITTED_FORMAT,
                                (unsigned int) studentId,
                                studentGpa,
                                admittedProgram);
    assert((lengthResult >= 0) && "snprintf() failure");

    size_t sizeResult = lengthResult + 1; /* add 1 for '\0' terminator */

    char *message = malloc(sizeResult);
    assert(message && "malloc() failure");

    /* build the message */
    assert(snprintf(message,
                    sizeResult,
                    ADMITTED_FORMAT,
                    (unsigned int) studentId,
                    studentGpa,
                    admittedProgram) == lengthResult);

    /* build the department name */
    char departmentName[] = "<Department#>";
    departmentName[sizeof(departmentName) - 3] = dep->letter; /* index of '#' */
    DEBUG_LOG("sending admission message \"%s\" to %s",
              message,
              departmentName);

    /* send the message */
    sendPhase2Message(phase2Socket,
                      departmentIp,
                      dep->port,
                      message,
                      "one admitted student",
                      departmentName);

    /* release the message */
    free(message);
}

static void handleStudent(int                student,
                          const AdmissionDb *aDb,
                          const uint32_t    *departmentIps)
{
    DEBUG_LOG("handling student %d", student);

    uint16_t studentId      = 0;
    double   studentGpa     = 0.0;
    uint16_t countInterests = 0;
    if (!receiveInitialStudentPacket(student,
                                     &studentId,
                                     &studentGpa,
                                     &countInterests)) {
        /* close() the TCP connection */
        assert(close(student) == 0);
        return;
    }
    char     *admittedProgram      = NULL;
    uint16_t  admittedDepartmentId = 0;
    uint16_t countValidPrograms = processApplication(student,
                                                     studentId,
                                                     studentGpa,
                                                     countInterests,
                                                     aDb,
                                                     &admittedProgram,
                                                     &admittedDepartmentId);
    /* send the reply to the student */
    sendApplicationReply(student, countValidPrograms);

    if (countValidPrograms == 0) {
        /* close() the TCP connection */
        assert(close(student) == 0);
        return;
    }

    /* save the student's IP from the connection */
    uint32_t studentIp = getIp(student);

    /* close() the TCP connection */
    assert(close(student) == 0);

    /* create a socket for sending results to student and possibly department */
    int phase2Socket = createSocket(SOCK_DGRAM);
    announceSocket("The admission office", " for Phase 2", phase2Socket);

    if (admittedProgram) {
        /* retrieve the department information */
        const struct Department *department
            = &DEPARTMENTS[admittedDepartmentId - 1];

        /* send the student their acceptance message */
        sendStudentAccepted(phase2Socket,
                            studentIp,
                            studentId,
                            admittedProgram,
                            department->letter);

        /* retrieve the department IP address */
        uint32_t departmentIp = departmentIps[admittedDepartmentId - 1];

        /* send the department a notification of the student's acceptance */
        sendDepartmentAdmission(phase2Socket,
                                departmentIp,
                                department,
                                studentId,
                                studentGpa,
                                admittedProgram);
        free(admittedProgram);

    } else {
        /* send the student their rejection message */
        sendStudentRejected(phase2Socket,
                            studentIp,
                            studentId);
    }

    /* close() the UDP socket */
    assert(close(phase2Socket) == 0);
}

static void *runHandleStudent(void *arg)
{
    const struct StudentHandler *handler = (const struct StudentHandler *) arg;

    /* recv() their program info and store it in the DB */
    handleStudent(handler->student,
                  handler->aDb,
                  handler->departmentIps);
    
    return NULL;
}

/**
 * @brief TODO
 */
static void admissionPhase2(int                admission,
                            const AdmissionDb *aDb,
                            const uint32_t    *departmentIps)
{
    DEBUG_LOG("starting Phase 2");

    /* allocate a thread and memory to hold arguments for all COUNT_DEPARTMENTS
     * expected Department clients */
    struct StudentHandler *handlers = malloc(
        COUNT_STUDENTS * sizeof(struct StudentHandler)
    );
    assert(handlers && "malloc() failure");

    /* for the expected number of registered students... */
    size_t i = 0;
    for (; i < COUNT_STUDENTS; ++i) {
        DEBUG_LOG("waiting for application from next student...");

        /* accept a connection to the next student */
        int student = acceptClient(admission);

        DEBUG_LOG("accepted connection from student");

        /* set the handler arguments */
        struct StudentHandler *handler = &handlers[i];
        handler->student       = student;
        handler->aDb           = aDb;
        handler->departmentIps = departmentIps;

        /* handle the Student in a separate thread
         * s.t. clients can be handled concurrently */
        assert(pthread_create(&handler->thread,
                              NULL, /* no pthread attributes */
                              &runHandleStudent,
                              handler) == 0);
    }

    /* join the threads */
    for (i = 0; i < COUNT_STUDENTS; ++i) {
        assert(pthread_join(handlers[i].thread,
                            NULL /* discard retval */) == 0);
    }

    /* free the handlers once all Students have been served */
    free(handlers);

    atomicPrintf("End of Phase 2 for the admission office\n");
}

static void sendAdmissionsDoneTo(int      admissionsDone,
                                 uint32_t departmentIp,
                                 uint16_t departmentPort)
{
	struct sockaddr_in departmentAddress;
    (void) memset(&departmentAddress, 0, sizeof(departmentAddress));
	departmentAddress.sin_family      = AF_INET;
	departmentAddress.sin_port        = htons(departmentPort);
	departmentAddress.sin_addr.s_addr = departmentIp;

    static const uint16_t DONE_MESSAGE = 0;
    assert(sendto(admissionsDone,
                  &DONE_MESSAGE,
                  sizeof(DONE_MESSAGE),
                  0,
                  (const struct sockaddr *) &departmentAddress,
                  sizeof(departmentAddress)) == sizeof(DONE_MESSAGE));
}

static void sendAdmissionsDone(const uint32_t *departmentIps)
{
    /* create a UDP socket */
    int admissionsDone = createSocket(SOCK_DGRAM);

    size_t i = 0;
    for (; i < COUNT_DEPARTMENTS; ++i) {
        /* tell all waiting departments that no more admissions will be sent */
        sendAdmissionsDoneTo(admissionsDone,
                             departmentIps[i],
                             DEPARTMENTS[i].port);
    }

    /* close() the UDP socket */
    assert(close(admissionsDone) == 0);
}

int main()
{
    /* create the TCP server socket */
    int admission = createAdmissionSocket();

    /* announce TCP port and IP address */
    announceSocket("The admission office", "", admission);

    /* allocate a table of department IP addresses */
    uint32_t *departmentIps = malloc(  sizeof(*departmentIps)
                                     * COUNT_DEPARTMENTS);
    assert(departmentIps && "malloc() failure");

    /* complete phase 1 */
    AdmissionDb *aDb = admissionPhase1(admission, departmentIps);

    /* complete phase 2 */
    admissionPhase2(admission, aDb, departmentIps);

    /* tell the departments that the admissions process is over */
    sendAdmissionsDone(departmentIps);

    /* destroy the admissions database */
    aDbDestroy(aDb);

    /* free the department IP addresses */
    free(departmentIps);

    /* close the TCP server socket */
    assert(close(admission) == 0);

    return EXIT_SUCCESS;
}
