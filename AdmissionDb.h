#ifndef ADMISSION_TABLE_H
#define ADMISSION_TABLE_H

#include <stdint.h> /* uint16_t */

typedef struct AdmissionDb AdmissionDb;

AdmissionDb *aDbCreate();

int aDbAdd(AdmissionDb *aDb,
           const char  *program,
           double       minGpa,
           uint16_t     departmentId);

void aDbFinalize(AdmissionDb *aDb);

int aDbFind(const AdmissionDb *aDb,
            const char        *program,
            double            *minGpa,
            uint16_t          *departmentId);

void aDbDestroy(AdmissionDb *aDb);


#endif /* ifndef ADMISSION_TABLE_H */
