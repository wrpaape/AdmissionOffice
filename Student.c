#include <sys/types.h>  /* fork */
#include <sys/socket.h> /* send */
#include <unistd.h>     /* fork */
#include <sys/wait.h>   /* wait */
#include <stdlib.h>     /* malloc/free */
#include <string.h>     /* string utilities */
#include <assert.h>     /* assert */

#include "AdmissionCommon.h"  /* ADMISSION_PORT_NUMBER */
#include "AdmissionClient.h"  /* client utilties */
#include "StudentRegistrar.h" /* STUDENTS */

static uint16_t parseInput(FILE  *input,
                           char **gpa,
                           char **interests)
{
    static const char   GPA_KEY[]           = "GPA";
    static const char   INTEREST_KEY[]      = "Interest";
    static const size_t LENGTH_INTEREST_KEY = sizeof(INTEREST_KEY) - 1;

    *gpa = NULL;
    (void) memset(interests, 0, sizeof(*interests) * COUNT_MAX_INTERESTS);

    uint16_t countInterests = 0;
    char *key   = NULL;
    char *value = NULL;
    while (readConfig(input, ':', &key, &value)) {
        if (strncmp(key, INTEREST_KEY, LENGTH_INTEREST_KEY) == 0) {
            long interestNumber = strtol(key + LENGTH_INTEREST_KEY, NULL, 10);
            free(key);
            assert((interestNumber >= 1) && (interestNumber <= COUNT_MAX_INTERESTS));
            char **interest = &interests[interestNumber - 1];
            assert(!*interest && "Repeat interest provided");
            *interest = value;
            ++countInterests;

        } else if (strcmp(key, GPA_KEY) == 0) {
            assert(!*gpa && "Multiple GPAs provided");
            *gpa = value;

        } else {
            assert(0 && "Invalid Key");
        }
        free(key);
    }

    assert(*gpa && "No GPA provided");
    assert((countInterests > 0) && "No interests provided");
    return countInterests;
}

/**
 * @brief open a Student's input file in the current directory named
 *     "student<id>.txt"
 * @param[in] id the Student's id
 * @return the input file open for reading
 */
static FILE *openInputFile(uint16_t id)
{
    char inputPathname[sizeof("student65535.txt")]; /* max required size */
    assert(snprintf(inputPathname,
                    sizeof(inputPathname),
                    "student%u.txt",
                    (unsigned int) id) >= 0);
    FILE *input = fopen(inputPathname, "r");
    assert(input && "fopen() failure");
    return input;
}

static uint16_t readInput(uint16_t   id,
                          char     **gpa,
                          char     **interests)
{
    FILE *input = openInputFile(id);
    uint16_t countInterests = parseInput(input, gpa, interests);
    assert(fclose(input) == 0);
    return countInterests;
}

static size_t packApplicationPacket(uint16_t     id,
                                    const char  *payload,
                                    char       **buffer) 
{
    uint16_t lengthPayload = (uint16_t) strlen(payload);

    /* allocate a buffer big enough */
    size_t bufferSize = sizeof(id)
                      + sizeof(lengthPayload) + lengthPayload
                      + lengthPayload;

    char *bufferCursor = malloc(bufferSize);
    assert(bufferCursor && "malloc() failure");
    *buffer = bufferCursor;

    /* pack the application packet */
    bufferCursor = packShort(bufferCursor, id); /* student ID */
    (void)         packString(bufferCursor,
                              payload,
                              lengthPayload);   /* payload */
    return bufferSize;
}

static void sendApplicationPacket(int         admission,
                                  uint16_t    id,
                                  const char *payload)
{
    /* pack the information up into a single buffer */
    char  *buffer     = NULL;
    size_t bufferSize = packApplicationPacket(id, payload, &buffer);

    /* send it */
    assert(send(admission,
                buffer,
                bufferSize,
                0) == bufferSize);
    free(buffer);
}

static void sendApplication(int          admission,
                            uint16_t     id,
                            const char  *gpa,
                            char *const *interests,
                            uint16_t     countInterests)
{
    /* first send the GPA */
    sendApplicationPacket(admission, id, gpa);

    /* then send the list of interests */
    size_t i = 0;
    for (; i < countInterests; ++i) {
        sendApplicationPacket(admission, id, interests[i]);
    }
}

static uint16_t receiveAdmissionReply(int admission)
{
    return 0;
}

static uint16_t apply(uint16_t    id,
                      const char *name)
{
    char  *gpa        = NULL;
    char  **interests = calloc(sizeof(*interests), COUNT_MAX_INTERESTS);
    assert(interests && "calloc() failure");

    uint16_t countInterests = readInput(id, &gpa, interests);

    int admission = connectToAdmission(name, "");

    sendApplication(admission, id, gpa, interests, countInterests);

    free(interests);
    free(gpa);

    atomicPrintf("Completed sending application for %s.\n", name);

    uint16_t countValidPrograms = receiveAdmissionReply(admission);

    atomicPrintf("%s has received the reply from the admission office\n", name);

    assert(close(admission) == 0);

    return countValidPrograms;
}

static char *receiveApplicationResult(uint16_t    port,
                                      const char *name)
{
    int listener = openAdmissionListener(port, name);

    char *result = NULL;
    assert(receiveString(listener,
                         &result) && "no application result");

    return result;
}

static void student(uint16_t id)
{
    char name[(sizeof("<Student65535>"))]; /* max required size */
    assert(snprintf(name,
                    sizeof(name),
                    "<Student%u>",
                    (unsigned int) id) >= 0);
    uint16_t countValidPrograms = apply(id, name);

    if (countValidPrograms > 0) {
        uint16_t port = STUDENT_PORTS[id - 1];
        char *result  = receiveApplicationResult(port, name);
        atomicPrintf("%s has received the application result: %s\n",
                     name, result);
        free(result);
    }

    atomicPrintf("End of phase 2 for %s\n", name);
}

int main()
{
    uint16_t id = 1;
    for (; id <= COUNT_STUDENTS; ++id) {
        pid_t forkStatus = fork();
        if (forkStatus == 0) {
            /* child process (Student instance) */
            student(id);
            exit(EXIT_SUCCESS);
        }
        assert(forkStatus > 0);
    }

    /* wait for child processes */
    int exitStatus  = EXIT_SUCCESS;
    int childStatus = EXIT_SUCCESS;
    while (wait(&childStatus) < 0) {
        /* if any child processes fail (nonzero), exit with a nonzero status*/
        exitStatus |= childStatus;
    }

    return exitStatus;
}
