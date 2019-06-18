#include <sys/types.h>  /* socket */
#include <sys/socket.h> /* socket */
#include <unistd.h>     /* close */
#include <arpa/inet.h>  /* htons */
#include <stdlib.h>     /* strtod */
#include <stdio.h>      /* I/O */
#include <string.h>     /* memset */
#include <assert.h>     /* assert */

#include "AdmissionCommon.h"     /* ADMISSION_PORT_NUMBER */
#include "DepartmentRegistrar.h" /* DEPARTMENTS */
#include "AdmissionDb.h"         /* AdmissionDb */


/**
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
 * @brief accept() a TCP connection from a department
 * @param[in] admission the listen()ing admission office socket
 * @return the socket connection with the department
 */
static int acceptDepartment(int admission)
{
    struct sockaddr_in departmentAddress;
    socklen_t addressLength = sizeof(departmentAddress);
    (void) memset(&departmentAddress, 0, addressLength);
    int department = accept(admission,
                            (struct sockaddr *) &departmentAddress,
                            &addressLength);
    assert(department != -1);
    return department;
}

/**
 * @brief recv() all program info messages from a @p department and store it in
 *     the @p aDb
 * @param[in] department the socket connection to the department
 * @param[in] aDb        the accumulating database of program info
 */
static void handleDepartment(int          department,
                             AdmissionDb *aDb)
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
        assert(aDbAdd(aDb, program, minGpa, departmentId));
        free(program);
        prevDepartmentId = departmentId;
    }

    /* check that at least 1 program info message was sent */
    if (departmentId != 0) {
        char departmentLetter = DEPARTMENTS[departmentId - 1].letter;
        assert(printf(
            "Received the program list from <Department%c>\n",
            departmentLetter
        ) >= 0);
    }
}

/**
 * @brief builds an AdmissionDb of program info received by the registered
 *     departments
 * @return the DB of program info, keyed by program name
 */
static AdmissionDb *admissionPhase1()
{
    int admission = createAdmissionSocket();

    /* announce TCP port and IP address */
    announceConnection("The admission office", "", admission);

    AdmissionDb *aDb = aDbCreate();
    assert(aDb);

    /* for the expected number of registered departments... */
    size_t remDepartments = COUNT_DEPARTMENTS;
    for (; remDepartments > 0; --remDepartments) {
        /* accept a connection to the next department */
        int department = acceptDepartment(admission);

        /* recv() their program info and store it in the DB */
        handleDepartment(department,
                         aDb);

        /* close() the connection */
        assert(close(department) == 0);
    }

    /* ready the DB for lookup */
    aDbFinalize(aDb);

    assert(close(admission) == 0);

    assert(puts(
        "End of Phase 1 for the admission office"
    ) >= 0);

    return aDb;
}

/**
 * @brief TODO
 */
static void admissionPhase2(AdmissionDb *aDb)
{
    (void) aDb; /* suppress unused argument warning for now */
}


int main()
{
    AdmissionDb *aDb = admissionPhase1();

    admissionPhase2(aDb);

    aDbDestroy(aDb);
    return EXIT_SUCCESS;
}
