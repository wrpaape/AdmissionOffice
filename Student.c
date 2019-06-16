#include "Student.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ReadConfig.h"

static size_t parseInput(FILE  *input,
                         char **gpa,
                         char  *interests[MAX_INTERESTS])
{
    static const char   GPA_KEY[]            = "GPA";
    static const char   INTEREST_KEY[]       = "Interest";
    static const size_t LENGTH_INTEREST_KEY  = sizeof(INTEREST_KEY) - 1;

    *gpa = NULL;
    (void) memset(interests, 0, sizeof(*interests) * MAX_INTERESTS);

    size_t countInterests = 0;
    char *key   = NULL;
    char *value = NULL;
    while (readConfig(input, ':', &key, &value)) {
        if (strncmp(key, INTEREST_KEY, LENGTH_INTEREST_KEY) == 0) {
            long interestNumber = strtol(key + LENGTH_INTEREST_KEY, NULL, 10);
            free(key);
            assert((interestNumber >= 1) && (interestNumber <= MAX_INTERESTS));
            size_t interestIndex = interestNumber - 1;
            assert(!interests[interestIndex]);
            interests[interestIndex] = value;

        } else if (strcmp(key, GPA_KEY) == 0) {
            assert(!*gpa && "Multiple GPAs provided");
            *gpa = value;

        } else {
            assert(0 && "Invalid Key");
        }
        free(key);
    }

    assert(*gpa && "No GPA provided");
    assert(countInterests > 0);
    return countInterests;
}

static size_t readInput(const char *inputPathname,
                        char      **gpa,
                        char       *interests[MAX_INTERESTS])
{
    FILE *input = fopen(inputPathname, "r");
    assert(input);
    size_t countInterests = parseInput(input, gpa, interests);
    assert(fclose(input) == 0);
    return countInterests;
}

void student(const char *inputPathname)
{
    char  *gpa = NULL;
    char  *interests[MAX_INTERESTS] = { 0 };
    size_t countInterests = readInput(inputPathname, &gpa, interests);
}
