#ifndef ADMISSION_DEPARTMENT_INTERFACE_H
#define ADMISSION_DEPARTMENT_INTERFACE_H

#include <stdint.h> /* uint16_t */
#include <stddef.h> /* size_t */

/**
 * Contains the information registered to each of the Departments in the
 * Admission Office simulation.
 */

/**
 * Information for a Department instance.
 */
struct Department {
    char     letter; /** the department instance's letter */
    uint16_t port;   /** the department instance's static UDP port */
};

/**
 * The array of Department info registered in advance of starting the
 * simulation.  Each department instance can be identified by a 16-bit ID:
 * [1, COUNT_DEPARTMENTS], where DEPARTMENTS[<ID> - 1] will access the info
 * belonging to the Department with an ID of <ID>.
 */
extern const struct Department DEPARTMENTS[];

/**
 * The number of registered Department instances.
 */
extern const size_t COUNT_DEPARTMENTS;

#endif /* ifndef ADMISSION_DEPARTMENT_INTERFACE_H */
