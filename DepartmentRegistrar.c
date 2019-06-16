#include "DepartmentRegistrar.h"
#include "IdDigits.h"

const struct Department DEPARTMENTS[] = {
   /* ID: 1 */ { 'A', 2110 + ID_DIGITS },
   /* ID: 2 */ { 'B', 2120 + ID_DIGITS },
   /* ID: 3 */ { 'C', 2130 + ID_DIGITS }
};
const size_t COUNT_DEPARTMENTS = sizeof(DEPARTMENTS) / sizeof(*DEPARTMENTS);
