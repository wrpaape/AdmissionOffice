#include "AdmissionDb.h"

#include <stdlib.h> /* *alloc, free, qsort */
#include <string.h> /* string utilities */

struct AdmissionNode {
    char     *program;
    double    minGpa;
    uint16_t  departmentId;
};

struct AdmissionDb {
    size_t                count;
    struct AdmissionNode *nodes;
    size_t                capacity;
};

AdmissionDb *aDbCreate()
{
    return calloc(sizeof(struct AdmissionDb), 1); /* all zeros */
}

static int ensureCapacity(AdmissionDb *aDb)
{
    size_t nextCount = aDb->count + 1;
    if (nextCount > aDb->capacity) {
        size_t nextCapacity = nextCount * 2;
        struct AdmissionNode *nextNodes = realloc(
            aDb->nodes,
            sizeof(*aDb->nodes) * nextCapacity
        );
        if (!nextNodes) {
            return 0;
        }
        aDb->nodes    = nextNodes;
        aDb->capacity = nextCapacity;
    }
    return 1;
}

int aDbAdd(AdmissionDb *aDb,
           const char  *program,
           double       minGpa,
           uint16_t     departmentId)
{
    char *programCopy = strdup(program);

    if (programCopy && ensureCapacity(aDb)) {
        struct AdmissionNode *node = &aDb->nodes[aDb->count++];
        node->program      = programCopy;
        node->minGpa       = minGpa;
        node->departmentId = departmentId;
        return 1;
    }

    free(programCopy);
    return 0;
}

static int compareNodes(const void *lhs, const void *rhs)
{
    const struct AdmissionNode *lhsNode = (const struct AdmissionNode *) lhs;
    const struct AdmissionNode *rhsNode = (const struct AdmissionNode *) rhs;
    return strcmp(lhsNode->program,
                  rhsNode->program);
}

void aDbFinalize(AdmissionDb *aDb)
{
    qsort(aDb->nodes, aDb->count, sizeof(*aDb->nodes), &compareNodes);
}

const struct AdmissionNode *findNode(const struct AdmissionNode *begin,
                                     const struct AdmissionNode *end,
                                     const char                 *program)
{
    while (begin < end) {
        size_t length                   = end - begin;
        const struct AdmissionNode *mid = begin + (length / 2);

        int comparison = strcmp(program, mid->program);
        if (comparison < 0) {
            end = mid;
        } else if (comparison > 0) {
            begin = mid + 1;
        } else {
            return mid;
        }
    }
    return NULL;
}

int aDbFind(const AdmissionDb *aDb,
            const char        *program,
            double            *minGpa,
            uint16_t          *departmentId)
{
    const struct AdmissionNode *node = findNode(aDb->nodes,
                                                aDb->nodes + aDb->count,
                                                program);
    if (node) {
        *minGpa       = node->minGpa;
        *departmentId = node->departmentId;
        return 1;
    }
    return 0;
}

void aDbDestroy(AdmissionDb *aDb)
{
    struct AdmissionNode *node     = aDb->nodes;
    struct AdmissionNode *nodesEnd = aDb->nodes + aDb->count;
    for (; node < nodesEnd; ++node) {
        free(node->program);
    }
    free(aDb->nodes);
    free(aDb);
}
