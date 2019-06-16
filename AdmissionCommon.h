#ifndef ADMISSION_COMMON_H
#define ADMISSION_COMMON_H

#include <stdint.h> /* uint16_t */

extern const uint16_t ADMISSION_PORT_NUMBER;

void announceConnection(const char *name,
                        const char *trailer,
                        int         socket);

#endif /* ifndef ADMISSION_COMMON_H */
