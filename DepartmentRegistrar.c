#include "DepartmentRegistrar.h"
#include "IdDigits.h"

const struct Department DEPARTMENTS[] = {
   /* ID: 1 */ { 'A', 21100 + ID_DIGITS },
   /* ID: 2 */ { 'B', 21200 + ID_DIGITS },
   /* ID: 3 */ { 'C', 21300 + ID_DIGITS }
};
const size_t COUNT_DEPARTMENTS = sizeof(DEPARTMENTS) / sizeof(*DEPARTMENTS);
