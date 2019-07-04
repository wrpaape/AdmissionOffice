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
    int              department; /** the department connection socket */
    AdmissionDb     *aDb;        /** the accumulating DB of program info */
    pthread_mutex_t *aDbLock;    /** the lock to synchronize access to aDb */
    pthread_t        thread;     /** the thread handle */
};

/**
 * A collection of arguments and data structures needed to handle a Student
 * connection in Phase 2.
 */
struct StudentHandler {
    int                student; /** the student connection socket */
    const AdmissionDb *aDb;     /** the read-only DB of program info */
    pthread_t          thread;  /** the thread handle */
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
    int admission = socket(AF_INET, SOCK_STREAM, 0);
    assert(admission != -1);

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
 * @brief recv() a 16-bit short off of a @p socket
 * @param[in]  socket  the socket
 * @param[out] integer the result
 * @return non-zero if successful
 */
static int receiveShort(int       socket,
                        uint16_t *integer)
{
    uint16_t recvInteger = 0;
    if (recv(socket,
             &recvInteger,
             sizeof(recvInteger),
             0) != sizeof(recvInteger)) {
        return 0;
    }

    *integer = ntohs(recvInteger);
    return 1;
}

/**
 * @brief recv() a string off of a @p socket packed accordingly:
 *     <length:uint16_t>
 *     <byte_1>
 *     <byte_2>
 *     ... 
 *     <byte_length>
 * @param[in]  socket the socket
 * @param[out] string the result
 * @return non-zero if successful
 */
static int receiveString(int    socket,
                         char **string)
{
    /* recv() the length */
    uint16_t length = 0;
    if (!receiveShort(socket,
                      &length)) {
        return 0;
    }

    /* allocate room for the string + '\0' */
    char *recvString = malloc(length + 1);
    if (!recvString) {
        return 0;
    }

    /* recv() the actual string */
    if (recv(socket,
             recvString,
             length,
             0) != length) {
        free(recvString);
        return 0;
    }

    recvString[length] = '\0'; /* terminate string */
    *string = recvString;
    return 1;
}

/**
 * @brief recv() the minimum GPA portion of the programming info message
 * @param[in]  socket the socket
 * @param[out] minGpa the result
 * @return non-zero if successful
 */
static int receiveMinGpa(int     socket,
                         double *minGpa)
{
    /* GPA is communicated as an ASCII string over the network - read in the
     * raw string */
    char *minGpaString = NULL;
    if (!receiveString(socket,
                       &minGpaString)) {
        return 0;
    }

    /* attempt to parse the double value from it */
    char *conversionEnd = NULL;
    *minGpa = strtod(minGpaString, &conversionEnd);

    int conversionSucceeded = (conversionEnd != minGpaString);
    free(minGpaString);
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
               && receiveMinGpa(department, minGpa);          /* recv() minimum acceptable GPA */

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
    struct sockaddr_in departmentAddress;
    socklen_t addressLength = sizeof(departmentAddress);
    (void) memset(&departmentAddress, 0, addressLength);
    int client = accept(admission,
                        (struct sockaddr *) &departmentAddress,
                        &addressLength);
    assert(client != -1);
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
                             pthread_mutex_t *aDbLock)
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
            assert(departmentId == prevDepartmentId);
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

    /* close() the connection */
    assert(close(department) == 0);

    /* check that at least 1 program info message was sent */
    if (departmentId != 0) {
        char departmentLetter = DEPARTMENTS[departmentId - 1].letter;
        atomicPrintf("Received the program list from <Department%c>\n",
                     departmentLetter);
    }
}

static void *runHandleDepartment(void *arg)
{
    const struct DepartmentHandler *handler
        = (const struct DepartmentHandler *) arg;

    /* recv() their program info and store it in the DB */
    handleDepartment(handler->department,
                     handler->aDb,
                     handler->aDbLock);
    
    return NULL;
}

/**
 * @brief builds an AdmissionDb of program info received by the registered
 *     departments
 * @return the DB of program info, keyed by program name
 */
static AdmissionDb *admissionPhase1(int admission)
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
        handler->department = department;
        handler->aDb        = aDb;
        handler->aDbLock    = &aDbLock;

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

static void handleStudent(int                student,
                          const AdmissionDb *aDb)
{
}

static void *runHandleStudent(void *arg)
{
    const struct StudentHandler *handler = (const struct StudentHandler *) arg;

    /* recv() their program info and store it in the DB */
    handleStudent(handler->student,
                  handler->aDb);
    
    return NULL;
}

/**
 * @brief TODO
 */
static void admissionPhase2(int                admission,
                            const AdmissionDb *aDb)
{
    /* allocate a thread and memory to hold arguments for all COUNT_DEPARTMENTS
     * expected Department clients */
    struct StudentHandler *handlers = malloc(
        COUNT_STUDENTS * sizeof(struct StudentHandler)
    );
    assert(handlers && "malloc() failure");

    /* for the expected number of registered students... */
    size_t i = 0;
    for (; i < COUNT_STUDENTS; ++i) {
        /* accept a connection to the next department */
        int student = acceptClient(admission);

        /* set the handler arguments */
        struct StudentHandler *handler = &handlers[i];
        handler->student = student;
        handler->aDb     = aDb;

        /* handle the Student in a separate thread
         * s.t. clients can be handled concurrently */
        assert(pthread_create(&handler->thread,
                              NULL, /* no pthread attributes */
                              &runHandleStudent,
                              handler) == 0);
    }

    /** TODO: remove */
    /* join the threads */
    for (i = 0; i < COUNT_STUDENTS; ++i) {
        assert(pthread_join(handlers[i].thread,
                            NULL /* discard retval */) == 0);
    }

    /* free the handlers once all Students have been served */
    free(handlers);
}


int main()
{
    /* create the TCP server socket */
    int admission = createAdmissionSocket();

    /* announce TCP port and IP address */
    announceConnection("The admission office", "", admission);

    /* complete phase 1 */
    AdmissionDb *aDb = admissionPhase1(admission);

    /* complete phase 2 */
    admissionPhase2(admission, aDb);

    /* detroy the admissions database */
    aDbDestroy(aDb);

    /* close the TCP server socket */
    assert(close(admission) == 0);

    return EXIT_SUCCESS;
}
