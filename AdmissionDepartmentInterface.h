#ifndef ADMISSION_DEPARTMENT_INTERFACE_H
#define ADMISSION_DEPARTMENT_INTERFACE_H

#include <stdint.h> /* uint16_t */
#include <stddef.h> /* size_t */

struct Department {
    char     letter;
    uint16_t port;
};

extern const struct Department DEPARTMENTS[];
extern const size_t            COUNT_DEPARTMENTS;

#endif /* ifndef ADMISSION_DEPARTMENT_INTERFACE_H */
