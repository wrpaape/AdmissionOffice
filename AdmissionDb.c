#include "AdmissionDb.h"

#include <stdlib.h> /* *alloc, free, qsort */
#include <string.h> /* string utilities */

struct AdmissionNode {
    char     *program;      /* program name */
    double    minGpa;       /* the minimum acceptable GPA for this program */
    uint16_t  departmentId; /* ID of the program's department */
};

struct AdmissionDb {
    size_t                count;    /* current node count */
    struct AdmissionNode *nodes;    /* buffer of nodes */
    size_t                capacity; /* total buffer capacity */
};

AdmissionDb *aDbCreate()
{
    return calloc(sizeof(struct AdmissionDb), 1); /* all zeros */
}

/**
 * @brief check that the nodes buffer has enough capacity for an additional
 *     node
 * @return non-zero on success, zero on an allocation failure
 */
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
    /* make a deep copy of program s.t. we don't depend on caller maintaining
     * original */
    char *programCopy = strdup(program);

    if (programCopy && ensureCapacity(aDb)) {
        /* insert new node at the end of nodes buffer */
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
    /* sort the nodes buffer so that a binary search can be preformed for
     * lookup */
    qsort(aDb->nodes, aDb->count, sizeof(*aDb->nodes), &compareNodes);
}

static const struct AdmissionNode *findNode(const struct AdmissionNode *begin,
                                            const struct AdmissionNode *end,
                                            const char                 *program)
{
    /* binary search */
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
    int foundNode = (node != NULL);
    if (foundNode) {
        *minGpa       = node->minGpa;
        *departmentId = node->departmentId;
    }
    return foundNode;
}

void aDbDestroy(AdmissionDb *aDb)
{
    /* free all the copied program names */
    struct AdmissionNode *node     = aDb->nodes;
    struct AdmissionNode *nodesEnd = aDb->nodes + aDb->count;
    for (; node < nodesEnd; ++node) {
        free(node->program);
    }
    /* free the nodes buffer */
    free(aDb->nodes);
    /* free the AdmissionDb struct */
    free(aDb);
}
