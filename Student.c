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
            assert(   (interestNumber >= 1)
                   && ((size_t) interestNumber <= COUNT_MAX_INTERESTS));
            char **interest = &interests[interestNumber - 1];
            assert(!*interest && "repeat interest provided");
            *interest = value;
            ++countInterests;

        } else if (strcmp(key, GPA_KEY) == 0) {
            assert(!*gpa && "multiple GPAs provided");
            *gpa = value;

        } else {
            assert(!"invalid Key");
        }
        free(key);
    }

    assert(*gpa && "no GPA provided");
    assert((countInterests > 0) && "no interests provided");
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

static size_t packInitialPacket(uint16_t     id,
                                const char  *gpa,
                                uint16_t     countInterests,
                                char       **buffer) 
{
    uint16_t lengthGpa = (uint16_t) strlen(gpa);

    /* allocate a buffer big enough */
    size_t bufferSize = sizeof(id)
                      + sizeof(lengthGpa)
                      + lengthGpa
                      + sizeof(countInterests);

    char *bufferCursor = malloc(bufferSize);
    assert(bufferCursor && "malloc() failure");
    *buffer = bufferCursor;

    /* pack the initial packet */
    bufferCursor = packShort(bufferCursor, id); /* student ID */
    bufferCursor = packString(bufferCursor,
                              gpa,
                              lengthGpa);       /* GPA */
    (void)         packShort(bufferCursor,
                             countInterests);   /* number of interests */
    return bufferSize;

}

static void sendInitialPacket(int         admission,
                              uint16_t    id,
                              const char *gpa,
                              uint16_t    countInterests)
{
    /* pack the information up into a single buffer */
    char  *buffer     = NULL;
    size_t bufferSize = packInitialPacket(id, gpa, countInterests, &buffer);

    /* send it */
    assert(send(admission,
                buffer,
                bufferSize,
                0) == (ssize_t) bufferSize);

    /* free the temporary buffer */
    free(buffer);
}

static size_t packInterest(uint16_t     id,
                           const char  *interest,
                           char       **buffer) 
{
    uint16_t lengthInterest = (uint16_t) strlen(interest);

    /* allocate a buffer big enough */
    size_t bufferSize = sizeof(id)
                      + sizeof(lengthInterest)
                      + lengthInterest;

    char *bufferCursor = malloc(bufferSize);
    assert(bufferCursor && "malloc() failure");
    *buffer = bufferCursor;

    /* pack the interest packet */
    bufferCursor = packShort(bufferCursor, id); /* student ID */
    (void)         packString(bufferCursor,
                              interest,
                              lengthInterest);   /* interest */
    return bufferSize;
}

static void sendInterest(int         admission,
                         uint16_t    id,
                         const char *interest)
{
    /* pack the information up into a single buffer */
    char  *buffer     = NULL;
    size_t bufferSize = packInterest(id, interest, &buffer);

    /* send it */
    assert(send(admission,
                buffer,
                bufferSize,
                0) == (ssize_t) bufferSize);

    /* free the temporary buffer */
    free(buffer);
}

static void sendApplication(int          admission,
                            uint16_t     id,
                            const char  *gpa,
                            char *const *interests,
                            uint16_t     countInterests)
{
    DEBUG_LOG("student%u sending ID and GPA", (unsigned int) id);
    /* first send the GPA and number of interests */
    sendInitialPacket(admission, id, gpa, countInterests);

    /* then send the list of interests */
    size_t i = 0;
    for (; i < countInterests; ++i) {
        DEBUG_LOG("student%u sending interest: %s",
                  (unsigned int) id,
                  interests[i]);
        sendInterest(admission, id, interests[i]);
    }
}

static uint16_t receiveAdmissionReply(int admission)
{
    DEBUG_LOG("awaiting application reply...");
    uint16_t countValidPrograms = 0;
    assert(   receiveShort(admission, &countValidPrograms)
           && "received no reply from application");
    return countValidPrograms;
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

    DEBUG_LOG("%s - %u/%u programs are valid",
              name,
              (unsigned int) countValidPrograms,
              (unsigned int) countInterests);

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
        atomicPrintf("%s has received the application result\n", name);
        DEBUG_LOG("%s application result: \"%s\"",
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
        /* if any child processes fail (nonzero), exit with a nonzero status */
        exitStatus |= childStatus;
    }

    return exitStatus;
}
