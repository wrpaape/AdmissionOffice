#ifndef STUDENT_REGISTRAR_H
#define STUDENT_REGISTRAR_H

#include <stdint.h> /* uint16_t */
#include <stddef.h> /* size_t */

/**
 * The static UDP ports assigned to each student.
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
