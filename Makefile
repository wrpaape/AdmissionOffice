# DEBUG_FLAG =
DEBUG_FLAG = -DDEBUG
CC = gcc
CFLAGS = -Wall -Wextra -Werror -O0 -g -c -pthread $(DEBUG_FLAG)
LDFLAGS = -lnsl -lresolv -pthread
TARFLAGS = cvf
PACKAGE_BASE = ee450_paape_phase

PACKAGE1_CONTENTS = Admission.c \
                    Department.c \
                    AdmissionDb.c \
                    AdmissionDb.h \
                    AdmissionDbTest.c \
                    DepartmentRegistrar.c \
                    DepartmentRegistrar.h \
                    AdmissionClient.c \
                    AdmissionClient.h \
                    AdmissionCommon.c \
                    AdmissionCommon.h \
		    IdDigits.h \
		    Makefile \
                    readme.txt

PACKAGE2_CONTENTS = $(PACKAGE1_CONTENTS) \
                    Student.c \
                    StudentRegistrar.c \
                    StudentRegistrar.h

.PHONY: all test package1 package2 clean clean-all

all:  AdmissionDbTest Admission Department Student

package1: $(PACKAGE1_CONTENTS)
	tar $(TARFLAGS) $(PACKAGE_BASE)1.tar $^
	gzip $(PACKAGE_BASE)1.tar

package2: $(PACKAGE2_CONTENTS)
	tar $(TARFLAGS) $(PACKAGE_BASE)2.tar $^
	gzip $(PACKAGE_BASE)2.tar

test: AdmissionDbTest
	./$<

AdmissionDbTest: AdmissionDbTest.o AdmissionDb.o
	$(CC) $(LDFLAGS) $^ -o $@
AdmissionDbTest.o: AdmissionDbTest.c AdmissionDb.h
	$(CC) $(CFLAGS) $< -o $@

Department: Department.o DepartmentRegistrar.o AdmissionClient.o AdmissionCommon.o
	$(CC) $(LDFLAGS) $^ -o $@
Department.o: Department.c DepartmentRegistrar.h AdmissionClient.h AdmissionCommon.h
	$(CC) $(CFLAGS) $< -o $@

Student: Student.o StudentRegistrar.o AdmissionClient.o AdmissionCommon.o
	$(CC) $(LDFLAGS) $^ -o $@
Student.o: Student.c StudentRegistrar.h AdmissionClient.h AdmissionCommon.h
	$(CC) $(CFLAGS) $< -o $@

Admission: Admission.o AdmissionDb.o DepartmentRegistrar.o StudentRegistrar.o AdmissionCommon.o
	$(CC) $(LDFLAGS) $^ -o $@
Admission.o: Admission.c AdmissionDb.h DepartmentRegistrar.h StudentRegistrar.h AdmissionCommon.h
	$(CC) $(CFLAGS) $< -o $@
AdmissionDb.o: AdmissionDb.c AdmissionDb.h
	$(CC) $(CFLAGS) $< -o $@


DepartmentRegistrar.o: DepartmentRegistrar.c DepartmentRegistrar.h IdDigits.h
	$(CC) $(CFLAGS) $< -o $@

StudentRegistrar.o: StudentRegistrar.c StudentRegistrar.h IdDigits.h
	$(CC) $(CFLAGS) $< -o $@

AdmissionClient.o: AdmissionClient.c AdmissionClient.h AdmissionCommon.h
	$(CC) $(CFLAGS) $< -o $@

AdmissionCommon.o: AdmissionCommon.c AdmissionCommon.h IdDigits.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) *.o Student Department Admission AdmissionDbTest

clean-all: clean
	$(RM) $(PACKAGE_BASE)1.tar.gz $(PACKAGE_BASE)2.tar.gz

