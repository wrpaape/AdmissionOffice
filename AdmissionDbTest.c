#include "AdmissionDb.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

struct TestValue {
    const char *program;
    double      minGpa;
    uint16_t    departmentId;
};

const struct TestValue TEST_VALUES[] = {
    { "program1",  3.00,  0 },
    { "program2",  3.10,  1 },
    { "program3",  3.20,  2 },
    { "program4",  3.30,  3 },
    { "program5",  3.40,  4 },
    { "program6",  3.50,  5 },
    { "program7",  3.60,  6 },
    { "program8",  3.70,  7 },
    { "program9",  3.80,  8 },
    { "program10", 3.90,  9 },
    { "program11", 3.10, 10 },
    { "program12", 3.11, 11 },
    { "program13", 3.12, 12 },
    { "program14", 3.13, 13 },
    { "program15", 3.14, 14 },
    { "program16", 3.15, 15 },
    { "program17", 3.16, 16 },
    { "program18", 3.17, 17 },
    { "program19", 3.18, 18 },
    { "program20", 3.19, 29 },
    { "program21", 3.20, 20 },
    { "program22", 3.21, 21 },
    { "program23", 3.22, 22 },
    { "program24", 3.23, 23 },
    { "program25", 3.24, 24 }
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
                      value->departmentId));
    }
    aDbFinalize(aDb);
    return aDb;
}

static int checkValueFound(AdmissionDb            *aDb,
                           const struct TestValue *value)
{
    double minGpa         = 0.0;
    uint16_t departmentId = 0;
    if (!aDbFind(aDb,
                 value->program,
                 &minGpa,
                 &departmentId)) {
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
    if (value->departmentId != departmentId) {
        (void) fprintf(stderr,
                       "aDbFind() - found departmentId (%u) != expected (%u)\n",
                       (unsigned int) departmentId,
                       (unsigned int) value->departmentId);
        failures += 1;
    }
    return failures;
}

static int checkProgramNotFound(AdmissionDb *aDb,
                                const char  *program)
{
    double unusedminGpa         = 0.0;
    uint16_t unusedDepartmentId = 0;
    int failure = 0;
    if (aDbFind(aDb,
                program,
                &unusedminGpa,
                &unusedDepartmentId)) {
        (void) fprintf(stderr,
                       "aDbFind() - found unexpected program (%s)\n",
                       program);
        failure = 1;
    }
    return failure;
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
