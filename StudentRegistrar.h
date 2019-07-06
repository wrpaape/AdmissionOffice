#ifndef STUDENT_REGISTRAR_H
#define STUDENT_REGISTRAR_H

#include <stdint.h> /* uint16_t */
#include <stddef.h> /* size_t */

/**
 * Contains the information registered to each of the Students in the Admission
 * Office simulation.
 */

/**
 * The static UDP ports assigned to each student.  The array of static UDP
 * ports registered for each Student in advance of starting the simulation.
 * Each student instance can be identified by a 16-bit ID: [1, COUNT_STUDENTS],
 * where STUDENT_PORTS[<ID> - 1] will access the port belonging to the Student
 * with an ID of <ID>.
 */
extern const uint16_t STUDENT_PORTS[];

/**
 * The number of registered Student instances.
 */
extern const size_t COUNT_STUDENTS;

/**
 * The maximum number of interests per Student.
 */
extern const size_t COUNT_MAX_INTERESTS;

#endif /* ifndef STUDENT_REGISTRAR_H */
