#include <sys/types.h>  /* socket */
#include <sys/socket.h> /* socket */
#include <unistd.h>     /* close */
#include <arpa/inet.h>  /* htons */
#include <stdlib.h>     /* strtod */
#include <stdio.h>      /* I/O */
#include <string.h>     /* memset */
#include <assert.h>     /* assert */

#include "AdmissionCommon.h"              /* ADMISSION_PORT_NUMBER */
#include "AdmissionDepartmentInterface.h" /* DEPARTMENTS */
#include "AdmissionDb.h"                  /* AdmissionDb */


const int ADMISSION_BACKLOG = 128;

static int
createAdmissionSocket()
{
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

	struct sockaddr_in address;
    (void) memset(&address, 0, sizeof(address));
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port        = htons(ADMISSION_PORT_NUMBER);
    assert(bind(admission,
                (const struct sockaddr *) &address,
                sizeof(address)) == 0);

    assert(listen(admission, ADMISSION_BACKLOG) == 0);
    
    return admission;
}

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

static int receiveString(int    socket,
                         char **string)
{
    uint16_t length = 0;
    if (!receiveShort(socket,
                      &length)) {
        return 0;
    }

    char *recvString = malloc(length + 1);
    if (!recvString) {
        return 0;
    }

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

static int receiveMinGpa(int     socket,
                         double *minGpa)
{
    char *minGpaString = NULL;
    if (!receiveString(socket,
                       &minGpaString)) {
        return 0;
    }

    char *conversionEnd = NULL;
    double recvMinGpa = strtod(minGpaString, &conversionEnd);

    int conversionSucceeded = (minGpaString != conversionEnd);
    free(minGpaString);
    return conversionSucceeded;
}

static int receiveDepartmentInfo(int       department,
                                 char    **program,
                                 double   *minGpa,
                                 uint16_t *departmentId)
{
    char *recvProgram         = NULL;
    uint16_t recvDepartmentId = 0;
    int success = receiveShort(department,  &recvDepartmentId)
               && (recvDepartmentId >= 1)
               && (recvDepartmentId <= COUNT_DEPARTMENTS)
               && receiveString(department, &recvProgram)
               && receiveMinGpa(department, minGpa);

    if (success) {
        *program      = recvProgram;
        *departmentId = recvDepartmentId;
    } else {
        free(recvProgram);
    }

    return success;
}

static int acceptDepartment(int                 admission,
                            struct sockaddr_in *departmentAddress)
{
    socklen_t addressLength = sizeof(*departmentAddress);
    (void) memset(departmentAddress, 0, addressLength);
    int department = accept(admission,
                            (struct sockaddr *) departmentAddress,
                            &addressLength);
    assert(department != -1);
    return department;
}

static void handleDepartment(int                       department,
                             const struct sockaddr_in *departmentAddress,
                             AdmissionDb              *aDb)
{
    char     *program          = NULL;
    double    minGpa           = 0.0;
    uint16_t  departmentId     = 0;
    uint16_t  prevDepartmentId = 0;
    while (receiveDepartmentInfo(department,
                                 &program,
                                 &minGpa,
                                 &departmentId)) {
        if (prevDepartmentId != 0) {
            assert(departmentId == prevDepartmentId);
        }
        assert(aDbAdd(aDb, program, minGpa, departmentId));
        free(program);
        prevDepartmentId = departmentId;
    }

    assert(departmentId != 0);

    char departmentLetter = DEPARTMENTS[departmentId - 1].letter;
    assert(printf(
        "Received the program list from Department%c\n",
        departmentLetter
    ) >= 0);

}

static AdmissionDb *admissionPhase1()
{
    int admission = createAdmissionSocket();
    announceConnection("The admission office", "", admission);

    AdmissionDb *aDb = aDbCreate();
    assert(aDb);

    size_t remDepartments = COUNT_DEPARTMENTS;
    for (; remDepartments > 0; --remDepartments) {
        struct sockaddr_in departmentAddress;
        int department = acceptDepartment(admission,
                                          &departmentAddress);
        handleDepartment(department,
                         &departmentAddress,
                         aDb);

        assert(close(department) == 0);
    }


    aDbFinalize(aDb);

    assert(close(admission) == 0);

    assert(puts(
        "End of Phase 1 for the admission office"
    ) >= 0);

    return aDb;
}


int main()
{
    AdmissionDb *aDb = admissionPhase1();

    aDbDestroy(aDb);
    return EXIT_SUCCESS;
}
