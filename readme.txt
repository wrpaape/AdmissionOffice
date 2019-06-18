# EE450 Socket Programming Project (Phase 1)

## A. Name:       William Reid Paape
## B. Student ID: 2877379718
##    NetID:      paape

## C. What

## D. Code Files

| File(s)                   | Purpose                                          |
|-------------------------- | ------------------------------------------------ |
| Admission.c               | The `Admission` Office process.  this module     |
|                           | starts the `Admission` server, stores program    |
|                           | information provided by the `Department`s, and   |
|                           | then handles incoming `Student` applications.    |
| Department.c              | The `Deparment` processess.  This module         |
|                           | launches multiple concurrent `Department`        |
|                           | instances which will read their program          |
|                           | information from a configuration file, relay     |
|                           | that information to the `Admission` server, and  |
|                           | then in phase 2 receive notification of admitted |
|                           | students.                                        |
| AdmissionDb.{c,h}         | The in-memory database of program info keyed by  |
|                           | program name.  The `Admission` module leverages  |
|                           | this container to store and find program         |
|                           | information for handling `Student` applications  |
| AdmissionDbTest.c         | Tests the correctness of the `AdmissionDb`       |
|                           | module.                                          |
| DepartmentRegistrar.{c,h} | Contains the information registered to each of   |
|                           | the Departments in the Admission Office          |
|                           | simulation.                                      |
| AdmissionClient.{c,h}     | Contains code common to TCP clients of the       |
|                           | `Admission` server.  Used by modules             |
|                           | `Department` and `Student` (phase 2)             |
| AdmissionCommon.{c,h}     | Contains utility code shared by all executables. |
| IdDigits.h                | The last 3 digits of my USC ID - needed for      |
|                           | allocating unique port numbers.                  |
| Makefile                  | Script for building the project executables.     |
| readme.txt                | Summarizes the project.                          |


E. Phase 1 Run Instructions
```
# on one terminal
./Admission
...
# on another terminal
./Department

# observe output from each terminal
```
For instance, a trial run of executing phase 1 may produce the following results:
```
# Admission instance
./Admission
The admission office has TCP port 4018 and IP address 127.0.0.1
Received the program list from <DepartmentA>
Received the program list from <DepartmentB>
Received the program list from <DepartmentC>
End of Phase 1 for the admission office

# Department instance
./Department
<DepartmentA> has TCP port 56832 and IP address 127.0.0.1 for Phase 1
<DepartmentA> is now connected to the admission office
<DepartmentB> has TCP port 56834 and IP address 127.0.0.1 for Phase 1
<DepartmentB> is now connected to the admission office
<DepartmentA> has sent <A1> to the admission office
<DepartmentB> has sent <B1> to the admission office
<DepartmentA> has sent <A2> to the admission office
<DepartmentB> has sent <B2> to the admission office
<DepartmentB> has sent <B3> to the admission office
<DepartmentA> has sent <A3> to the admission office
<DepartmentC> has TCP port 56836 and IP address 127.0.0.1 for Phase 1
<DepartmentC> is now connected to the admission office
Updating the admission office is done for <DepartmentB>
Updating the admission office is done for <DepartmentA>
<DepartmentC> has sent <C1> to the admission office
<DepartmentC> has sent <C2> to the admission office
<DepartmentC> has sent <C3> to the admission office
Updating the admission office is done for <DepartmentC>
```

## G. Code Idiosyncrasies
- any system call (memory allocation, I/O) failures result in a program abort()
- ill-formed messages may crash the program
- ill-formed messages are not NACKed
- the `Admission` office relies on `Department`
- the program relies on the name of departments being distinguishable by a single letter
