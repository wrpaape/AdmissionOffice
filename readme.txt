# EE450 Socket Programming Project (Phase 1)


## A. Name:       William Reid Paape
## B. Student ID: 2877379718
##    NetID:      paape


## C. What I Have Done in the Assignment
I have completed Phase 1 of the Admission Office Simulation Socket Programming
Project of EE450. In this phase, multiple 'Department's send their program
information to a central 'Admission' Office.


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


## F. Message Formats

### Program Information

#### Overview

This packet is sent from `Department` clients to the `Admission` office to
register their program information.  It contains a program name, the minimum
acceptable GPA for the program, and the unique ID of the sender `Department`.
A `Department` will send one of these packets for every line of their
configuration file.

#### Format
| Field               | Type         | Byte Offset               | Description |
|-------------------- | -------------|-------------------------- |------------ |
| Department ID       | `uint16_t`   | 0                         | The unique ID of a registered Department in the Simulation belonging to the range [1,`COUNT_DEPARTMENTS`) |
| Program Name Length | `uint16_t`   | 2                         | The string length of the Program Name (e.g. 2) |
| Program Name        | `char` array | 4                         | The Program Name (e.g. "A1") |
| Minimum GPA Length  | `uint16_t`   | 4 + [Program Name Length] | The string length of the Minimum GPA (e.g. 2) |
| Minimum GPA         | `char` array | 6 + [Program Name Length] | The ASCII string representation of the floating-point minimum acceptable GPA for the program (e.g. "3.8") |


## G. Code Idiosyncrasies
- any system call (memory allocation, I/O) failures result in a program abort()
- ill-formed messages may crash the program
- ill-formed messages are not NACKed
- the `Admission` office will wait indefinitely for `Department`s to send their program info before proceeding from phase 1
- the `Admission` office uses POSIX threads to concurrently handle clients and not fork()
- the program relies on the name of departments being distinguishable by a single letter
