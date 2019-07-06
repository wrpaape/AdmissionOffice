# EE450 Socket Programming Project (Phase 1)


## A. Name:       William Reid Paape
## B. Student ID: 2877379718
##    NetID:      paape


## C. What I Have Done in the Assignment
I have completed both Phases of the Admission Office Simulation Socket
Programming Project of EE450. In Phase 1, multiple 'Department's send their
program information to a central 'Admission' Office. In Phase 2, 'Student's
send their applications to the 'Admission' office. The admission office
receives these applications and processes them according to the program
information provided by the departments.  The 'Admission' office then sends an
"Accept" or "Reject" result to the 'Student' and notifies the 'Department's of
each accepted student.


## D. Code Files

| File(s)                   | Purpose                                          |
|-------------------------- | ------------------------------------------------ |
| Admission.c               | The `Admission` Office process.  this module     |
|                           | starts the `Admission` server, stores program    |
|                           | information provided by the `Department`s, and   |
|                           | then handles incoming `Student` applications.    |
|-------------------------- | ------------------------------------------------ |
| Department.c              | The `Deparment` processess.  This module         |
|                           | launches multiple concurrent `Department`        |
|                           | instances which will read their program          |
|                           | information from a configuration file, relay     |
|                           | that information to the `Admission` server, and  |
|                           | then in phase 2 receive notification of admitted |
|                           | students.                                        |
|-------------------------- | ------------------------------------------------ |
| Student.c                 | The `Student` processess. This module launches   |
|                           | multiple concurrent `Student` instances which    |
|                           | will read their application information from a   |
|                           | configuration file, relay that information to    |
|                           | the `Admission` server, and await their result.  |
|-------------------------- | ------------------------------------------------ |
| AdmissionDb.{c,h}         | The in-memory database of program info keyed by  |
|                           | program name.  The `Admission` module leverages  |
|                           | this container to store and find program         |
|                           | information for handling `Student` applications. |
|-------------------------- | ------------------------------------------------ |
| AdmissionDbTest.c         | Tests the correctness of the `AdmissionDb`       |
|                           | module.                                          |
|-------------------------- | ------------------------------------------------ |
| DepartmentRegistrar.{c,h} | Contains the information registered to each of   |
|                           | the `Department`s in the Admission Office        |
|                           | simulation.                                      |
|-------------------------- | ------------------------------------------------ |
| StudentRegistrar.{c,h}    | Contains the information registered to each of   |
|                           | the `Student`s in the Admission Office           |
|                           | simulation.                                      |
|-------------------------- | ------------------------------------------------ |
| AdmissionClient.{c,h}     | Contains code common to TCP clients of the       |
|                           | `Admission` server.  Used by modules             |
|                           | `Department` and `Student`.                      |
|-------------------------- | ------------------------------------------------ |
| AdmissionCommon.{c,h}     | Contains utility code shared by all executables. |
|-------------------------- | ------------------------------------------------ |
| IdDigits.h                | The last 3 digits of my USC ID - needed for      |
|                           | allocating unique port numbers.                  |
|-------------------------- | ------------------------------------------------ |
| Makefile                  | Script for building the project executables.     |
|-------------------------- | ------------------------------------------------ |
| readme.txt                | Summarizes the project.                          |
|-------------------------- | ------------------------------------------------ |


E. Phase 2 Run Instructions
```
# on one terminal
./Admission
...
# on another terminal
./Department
...
# on a third terminal
./Student

# observe output from each terminal
```
For instance, a trial run may produce the following results:
```
# Admission instance
The admission office has TCP port 4018 and IP address 127.0.0.1
Received the program list from <DepartmentA>
Received the program list from <DepartmentB>
Received the program list from <DepartmentC>
End of Phase 1 for the admission office
The admission office has TCP port 4018 and IP address 127.0.0.1
The admission office receive the application from <Student2>
The admission office receive the application from <Student1>
The admission office receive the application from <Student3>
The admission office receive the application from <Student4>
The admission office has UDP port 60121 and IP address 127.0.0.1 for Phase 2
The admission office has send the application result to <Student2>
The admission office has UDP port 36290 and IP address 127.0.0.1 for Phase 2
The admission office has UDP port 51452 and IP address 127.0.0.1 for Phase 2
The admission office has send one admitted student to <DepartmentA>
The admission office has UDP port 48186 and IP address 127.0.0.1 for Phase 2
The admission office has send the application result to <Student3>
The admission office has send the application result to <Student4>
The admission office has send the application result to <Student1>
The admission office has send one admitted student to <DepartmentB>
The admission office has send one admitted student to <DepartmentA>
The admission office receive the application from <Student5>
The admission office has UDP port 49850 and IP address 127.0.0.1 for Phase 2
The admission office has send the application result to <Student5>
The admission office has send one admitted student to <DepartmentC>
End of Phase 2 for the admission office

# Department instance
<DepartmentB> has TCP port 38474 and IP address 127.0.0.1 for Phase 1
<DepartmentA> has TCP port 38476 and IP address 127.0.0.1 for Phase 1
<DepartmentC> has TCP port 38480 and IP address 127.0.0.1 for Phase 1
<DepartmentB> is now connected to the admission office
<DepartmentA> is now connected to the admission office
<DepartmentC> is now connected to the admission office
<DepartmentB> has sent <B1> to the admission office
<DepartmentC> has sent <C1> to the admission office
<DepartmentA> has sent <A1> to the admission office
<DepartmentB> has sent <B2> to the admission office
<DepartmentC> has sent <C2> to the admission office
<DepartmentA> has sent <A2> to the admission office
<DepartmentB> has sent <B3> to the admission office
<DepartmentC> has sent <C3> to the admission office
<DepartmentA> has sent <A3> to the admission office
Updating the admission office is done for <DepartmentB>
End of Phase 1 for <DepartmentB>
Updating the admission office is done for <DepartmentC>
Updating the admission office is done for <DepartmentA>
<DepartmentB> has UDP port 21918 and IP address 127.0.0.1 for Phase 2
End of Phase 1 for <DepartmentC>
End of Phase 1 for <DepartmentA>
<DepartmentC> has UDP port 22018 and IP address 127.0.0.1 for Phase 2
<DepartmentA> has UDP port 21818 and IP address 127.0.0.1 for Phase 2
<Student2> has been admitted to <DepartmentA>
<Student4> has been admitted to <DepartmentB>
<Student1> has been admitted to <DepartmentA>
<Student5> has been admitted to <DepartmentC>
End of Phase 2 for <DepartmentA>
End of Phase 2 for <DepartmentB>
End of Phase 2 for <DepartmentC>

# Student instance
<Student2> has TCP port 38482 and IP address 127.0.0.1
<Student1> has TCP port 38484 and IP address 127.0.0.1
<Student3> has TCP port 38488 and IP address 127.0.0.1
<Student4> has TCP port 38490 and IP address 127.0.0.1
Completed sending application for <Student2>.
Completed sending application for <Student1>.
Completed sending application for <Student3>.
Completed sending application for <Student4>.
<Student2> has received the reply from the admission office
<Student1> has received the reply from the admission office
<Student3> has received the reply from the admission office
<Student4> has received the reply from the admission office
<Student1> has UDP port 22118 and IP address 127.0.0.1 for Phase 2
<Student1> has received the application result
<Student2> has UDP port 22218 and IP address 127.0.0.1 for Phase 2
End of phase 2 for <Student1>
<Student2> has received the application result
<Student4> has UDP port 22418 and IP address 127.0.0.1 for Phase 2
<Student3> has UDP port 22318 and IP address 127.0.0.1 for Phase 2
End of phase 2 for <Student2>
<Student4> has received the application result
<Student3> has received the application result
End of phase 2 for <Student4>
End of phase 2 for <Student3>
<Student5> has TCP port 38492 and IP address 127.0.0.1
Completed sending application for <Student5>.
<Student5> has received the reply from the admission office
<Student5> has UDP port 22518 and IP address 127.0.0.1 for Phase 2
<Student5> has received the application result
End of phase 2 for <Student5>
```

## F. Message Formats

### Department to Admission (TCP)

#### Program Information

##### Overview
This packet is sent from `Department` clients to the `Admission` office to
register their program information.  It contains a program name, the minimum
acceptable GPA for the program, and the unique ID of the sender `Department`.
A `Department` will send one of these packets for every line of their
configuration file.

##### Format
| Field              | Type         | Byte Offset          | Description |
|------------------- | -------------|--------------------- |------------ |
| Department ID      | `uint16_t`   | 0                    | The unique ID of a registered Department in the Simulation belonging to the range [1,`COUNT_DEPARTMENTS`) |
| Program Length     | `uint16_t`   | 2                    | The string length of the Program (e.g. "A1"'s 2) |
| Program            | `char` array | 4                    | The Program (e.g. "A1") |
| Minimum GPA Length | `uint16_t`   | 4 + [Program Length] | The string length of the Minimum GPA (e.g. "3.6"'s is 3) |
| Minimum GPA        | `char` array | 6 + [Program Length] | The ASCII string representation of the floating-point minimum acceptable GPA for the program (e.g. "3.8") |

### Admission to Department (UDP)

#### Program Admission Notification

##### Overview
This packet is sent from the `Admission` office the `Department`s whenever a
student is accepted into one of the `Department`'s programs. It consists of the
student's name, their GPA, and the program that they have been admitted to.
This information is transmitted in a single `'#'`-delimited string. Zero to one
packet for every `Student` will be sent to each `Department`.

##### Format
| Field   | Type         | Byte Offset | Description |
|-------- | -------------|------------ |------------ |
| Length  | `uint16_t`   | 0           | The non-zero string length of the Message |
| Message | `char` array | 2           | `"<Student Name>#<Student GPA>#<Admitted Program>"` |

#### Phase 2 Complete

##### Overview
This packet is sent from the `Admission` office the `Department`s once it has
processed all `Student` applications. It is a notification to stop listening
for `Program Admission`s. One of these packets will be sent to every
`Department`.

##### Format
| Field   | Type       | Byte Offset | Description |
|-------- | -----------|------------ |------------ |
| Done    | `uint16_t` | 0           | `0`         |

### Student to Admission (TCP)

#### Application Header

##### Overview
This packet is sent from `Student` clients to the `Admission` office as the
first packet of their application. It contains their `Student` ID, their GPA,
and the number of programs they will be applying to. One of these packets will
be sent per student. The `Student` is expected to follow the `Application
Header` with `Count Interest`x`Program Interest` packets (see below).

##### Format
| Field           | Type         | Byte Offset               | Description |
|---------------- | -------------|-------------------------- |------------ |
| Student ID      | `uint16_t`   | 0                         | The unique ID of a registered Student in the Simulation belonging to the range [1,`COUNT_STUDENTS`) |
| GPA Length      | `uint16_t`   | 2                         | The string length of the GPA (e.g. "3.8"'s is 3) |
| GPA             | `char` array | 4                         | The ASCII string representation of the Student's floating-point GPA |
| Count Interests | `uint16_t`   | 4 + [Program Name Length] | The number of programs (zero or more) that this student is interested in (will be applying to) |

#### Program Interest

##### Overview
This packet contains the name of a single program of interest in a `Student`'s
application. It consists of a `Student` ID followed by the name of the program.
`Count Interests`, specified in the preceeding `Application Header`, of these
packets will be expected from a `Student`. `Program Interests` shall be sent in
descending order of preference s.t. if a `Student` may be admitted to multiple
Programs, they will be admitted to the one sent earliest.

##### Format
| Field           | Type         | Byte Offset | Description |
|---------------- | -------------|------------ |------------ |
| Student ID      | `uint16_t`   | 0           | The unique ID of a registered Student in the Simulation belonging to the range [1,`COUNT_STUDENTS`) |
| Program Length  | `uint16_t`   | 2           | The string length of the Program (e.g. "A1"'s 2) |
| Program         | `char` array | 4           | The Program (e.g. "A1") |

### Admission to Student (TCP)

#### Application Reply

##### Overview
This packet is a single number indicating how many of the student's applied-to
programs actually exist. If at least one exists, the sending student can expect
a follow-up `Application Result` message.

##### Format
| Field                | Type       | Byte Offset | Description |
|--------------------- | -----------|------------ |------------ |
| Count Valid Programs | `uint16_t` | 0           | The number (zero or more) of applied programs that exist |

### Admission to Student (UDP)

#### Application Result

##### Overview
This packet is the result of a `Student`'s application. It is an indication of
acceptance to one of their programs of interest, along with the names of the
program and program department, or a message of rejection from all of their
programs of interest. As stated earlier, if a `Student` may be accepted to
multiple programs, the program applied to earliest will be selected by the
`Admission` office.

##### Format
| Field   | Type         | Byte Offset | Description |
|-------- | -------------|------------ |------------ |
| Length  | `uint16_t`   | 0           | The non-zero string length of the Message |
| Message | `char` array | 2           | `"Accepted#<Admitted Program>#<Admitted Program Department>"` if accepted or `"Rejected"` if rejected |


## G. Code Idiosyncrasies
- any system call (memory allocation, I/O) failures result in a program abort()
- ill-formed messages may crash the program
- ill-formed messages are not NACKed
- The correctness of the Minimum GPA field is not validated at the time a Department reads in the configuration file
- the `Admission` office will wait indefinitely for `Department`s to send their program info before proceeding from phase 1
- the `Admission` office uses POSIX threads to concurrently handle clients instead of fork()
- the program relies on the name of departments being distinguishable by a single letter


## H. Reused Code
- `Admission.c`'s `createAdmissionSocket()` routine is nearly identical to the
  TCP server socket setup routine create_server_socket() from my Lab 2
  assignment.  Both were written by me.
