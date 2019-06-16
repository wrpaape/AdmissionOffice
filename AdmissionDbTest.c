#include "AdmissionDb.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

struct TestValue {
    const char *program;
    double      minGpa;
    const char *department;
    uint16_t    port;
};

const struct TestValue TEST_VALUES[] = {
    { "program1",  3.00, "department1",   1 },
    { "program2",  3.10, "department2",   2 },
    { "program3",  3.20, "department3",   3 },
    { "program4",  3.30, "department4",   4 },
    { "program5",  3.40, "department5",   5 },
    { "program6",  3.50, "department6",   6 },
    { "program7",  3.60, "department7",   7 },
    { "program8",  3.70, "department8",   8 },
    { "program9",  3.80, "department9",   9 },
    { "program10", 3.90, "department10", 10 },
    { "program11", 3.10, "department11", 11 },
    { "program12", 3.11, "department12", 12 },
    { "program13", 3.12, "department13", 13 },
    { "program14", 3.13, "department14", 14 },
    { "program15", 3.14, "department15", 15 },
    { "program16", 3.15, "department16", 16 },
    { "program17", 3.16, "department17", 17 },
    { "program18", 3.17, "department18", 18 },
    { "program19", 3.18, "department19", 19 },
    { "program20", 3.19, "department20", 20 },
    { "program21", 3.20, "department21", 21 },
    { "program22", 3.21, "department22", 22 },
    { "program23", 3.22, "department23", 23 },
    { "program24", 3.23, "department24", 24 },
    { "program25", 3.24, "department25", 25 }
};
size_t COUNT_TEST_VALUES = sizeof(TEST_VALUES) / sizeof(*TEST_VALUES);


static AdmissionDb *makeDb()
{
    AdmissionDb *aDb = aDbCreate();
    assert(aDb);

    size_t i = 0;
    for (; i < COUNT_TEST_VALUES; ++i) {
        const struct TestValue *value = &TEST_VALUES[i];
        assert(aDbAdd(aDb,
                      value->program,
                      value->minGpa,
                      value->department,
                      value->port));
    }
    aDbFinalize(aDb);
    return aDb;
}

static int checkValueFound(AdmissionDb            *aDb,
                           const struct TestValue *value)
{
    double minGpa          = 0.0;
    const char *department = NULL;
    uint16_t port          = 0;
    if (!aDbFind(aDb,
                 value->program,
                 &minGpa,
                 &department,
                 &port)) {
        (void) fprintf(stderr,
                       "aDbFind() - failed to find expected program \"%s\"\n",
                       value->program);
        return 1;
    }
    int failures = 0;
    if (value->minGpa != minGpa) {
        (void) fprintf(stderr,
                       "aDbFind() - found minGpa (%f) != expected (%f)\n",
                       minGpa,
                       value->minGpa);
        failures += 1;
    }
    if (strcmp(value->department, department) != 0) {
        (void) fprintf(stderr,
                       "aDbFind() - found department (%s) != expected (%s)\n",
                       department,
                       value->department);
        failures += 1;
    }
    if (value->port != port) {
        (void) fprintf(stderr,
                       "aDbFind() - found port (%u) != expected (%u)\n",
                       (unsigned int) port,
                       (unsigned int) value->port);
        failures += 1;
    }
    return failures;
}

static int checkProgramNotFound(AdmissionDb *aDb,
                                const char  *program)
{
    double unusedminGpa          = 0.0;
    const char *unusedDepartment = NULL;
    uint16_t unusedPort          = 0;
    int failed = 0;
    if (aDbFind(aDb,
                program,
                &unusedminGpa,
                &unusedDepartment,
                &unusedPort)) {
        (void) fprintf(stderr,
                       "aDbFind() - found unexpected program (%s)\n",
                       program);
        failed = 1;
    }
    return failed;
}

int checkDb(AdmissionDb *aDb)
{
    int failures = 0;
    size_t i = 0;
    for (; i < COUNT_TEST_VALUES; ++i) {
        failures += checkValueFound(aDb, &TEST_VALUES[i]);
    }

    failures += checkProgramNotFound(aDb, "PhonyProgram1");
    failures += checkProgramNotFound(aDb, "PhonyProgram2");
    failures += checkProgramNotFound(aDb, "PhonyProgram3");

    const char *testStatus = failures ? "FAIL"  : "PASS";
    (void) printf("checkDb() - %s\n", testStatus);

    return failures;
}

int main()
{
    AdmissionDb *aDb = makeDb();

    int failures = 0;
    
    failures += checkDb(aDb);

    aDbDestroy(aDb);

    return failures;
}
