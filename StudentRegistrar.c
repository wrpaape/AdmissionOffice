#include "StudentRegistrar.h"
#include "IdDigits.h"

const uint16_t STUDENT_PORTS[] = {
   /* ID: 1 */ 21400 + ID_DIGITS,
   /* ID: 2 */ 21500 + ID_DIGITS,
   /* ID: 3 */ 21600 + ID_DIGITS,
   /* ID: 4 */ 21700 + ID_DIGITS,
   /* ID: 5 */ 21800 + ID_DIGITS
};
const size_t COUNT_STUDENTS = sizeof(STUDENT_PORTS) / sizeof(*STUDENT_PORTS);

const size_t COUNT_MAX_INTERESTS = 3;
