A. Name:       William Reid Paape
B. Student ID: 2877379718
   NetID:      paape

C. What

D. Code Files

| File(s)                   | Purpose                                          |
|-------------------------- | ------------------------------------------------ |
| Admission.c               | The `Admission` Office process:  this module
                              starts the `Admission` server, stores program
                              information provided by the `Department`s, and
                              then handles incoming `Student` applications.    |
| Department.c              | The `Deparment` processess.  This module launches
                              multiple concurrent `Department` instances which
                              will read their program information from a
                              configuration file, relay that information to the
                              `Admission` server, and then in phase 2 receive
                              notification of admitted students.               |
| AdmissionDb.{c,h}         | The in-memory database of program info keyed by
                              program name.  The `Admission` module leverages
                              this container to store and find program
                              information for handling `Student` applications  |
| AdmissionDbTest.c         | Tests the correctness of the `AdmissionDb`
                              module.                                          |
| DepartmentRegistrar.{c,h} | Contains the information registered to each of
                              the Departments in the Admission Office
                              simulation.                                      |
| AdmissionClient.{c,h}     | Contains code common to TCP clients of the
                              `Admission` server.  Used by modules `Department`
                              and `Student` (phase 2)                          |
| AdmissionCommon.{c,h}     | Contains utility code shared by all executables. |
| IdDigits.h                | The last 3 digits of my USC ID - needed for
                              allocating unique port numbers.                  |
| Makefile                  | Script for building the project executables.     |
| readme.txt                | Summarizes the project.                          |
