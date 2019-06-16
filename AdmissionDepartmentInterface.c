#include "AdmissionDepartmentInterface.h"
#include "IdDigits.h"

const struct Department DEPARTMENTS[] = {
    { 'A', 2110 + ID_DIGITS },
    { 'B', 2120 + ID_DIGITS },
    { 'C', 2130 + ID_DIGITS }
};
const size_t COUNT_DEPARTMENTS = sizeof(DEPARTMENTS) / sizeof(*DEPARTMENTS);
