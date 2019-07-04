#ifndef ADMISSION_TABLE_H
#define ADMISSION_TABLE_H

#include <stdint.h> /* uint16_t */

/**
 * An in-memory database of program info keyed by program name.  The DB is
 * built with successive calls to aDbAdd(), then after a call to aDbFinalize()
 * the DB can be searched.
 *
 * NOTE: the implementation works on the assumption that all inserted keys are
 *    unique - do not insert duplicate program names
 */
typedef struct AdmissionDb AdmissionDb;

/**
 * @brief allocates and initializes an AdmissionDb
 * @return a non-NULL instance of AdmissionDb on success, NULL if there was an
 *     allocation error
 */
AdmissionDb *aDbCreate();

/**
 * @brief insert program info into the DB
 * @param[in] aDb          the DB
 * @param[in] program      the name of the department program
 * @param[in] minGpa       the minimum GPA required by the program
 * @param[in] departmentId the ID of the department offering this program
 * @return a non-NULL instance of AdmissionDb on success, NULL if there was an
 *     allocation error
 */
int aDbAdd(AdmissionDb *aDb,
           const char  *program,
           double       minGpa,
           uint16_t     departmentId);

/**
 * @brief readies the DB for searching - additional calls to aDbAdd() will
 *     require another aDbFinalize() to make the DB searchable.
 * @param[out] aDb the DB
 */
void aDbFinalize(AdmissionDb *aDb);

/**
 * @brief searches the DB for the program info belonging to @p program
 * @param[in]  aDb          the DB
 * @param[in]  program      the name of the department program
 * @param[out] minGpa       the minimum GPA required by the program
 * @param[out] departmentId the ID of the department offering this program
 * @return 1 if info was found, 0 if @p program is not in the DB
 */
int aDbFind(const AdmissionDb *aDb,
            const char        *program,
            double            *minGpa,
            uint16_t          *departmentId);

/**
 * @brief deallocates the DB
 * @param[in] aDb the DB
 */
void aDbDestroy(AdmissionDb *aDb);

#endif /* ifndef ADMISSION_TABLE_H */
