# Concurrent-Workers

In this repository you will find two projects, **[Concurrent-Primes](#Concurrent-Primes)** and **[Salad-Making](#Salad-Makingr)**.
Both projects deal with multiple concurrent process that must be coordinated in order to progress towards a common goal. Briefly:
- The goal of Concurrent-Primes is to calculate prime numbers, while communicating via pipes. 
- Whereas the Salad-Making goal is to make some “virtual” salads, with the processes sharing the "virtual" ingredients on a shared memory segment, protected by semaphores.

*These projects were the programming projects 2 & 3 of the Operating System course at DIT – UoA. Credits to the professor of the course at the time,  [Alex Delis](https://www.alexdelis.eu/).*

# Table of Contents  
[Concurrent-Primes](#concurrent-primes)  
- [Summary](#summary)
- [Compilation](#compilation) 
- [Usage](#usage) 
- [Brief implementation overview](#brief-implementation-overview) 
-- [Process hierarchy](#process-hierarchy) 
- [Source code files overview](#source-code-files-overview) 
- [Output format](#output-format)
- [Useful notes](#useful-notes) 

[Salad-Making](#salad-making)  
- [Summary](#summary-1) 
- [Compilation](#compilation-1) 
- [Usage](#usage-1) 
- [Brief implementation overview](#brief-implementation-overview-1) 
- [Source code files overview](#source-code-files-overview-1) 
- [Output format](#output-format-1) 
- [Useful notes](#useful-notes-1)

# Concurrent-Primes

A hierarchy of processes that communicate in order to find prime numbers in C

## Summary:

This project focuses on creating and coordinating a dynamic number of processes utilizing the appropriate linux/unix system calls *(fork(), exec(), wait(), kill(), mkfifo(), pipe() etc)*. By using these system calls, it creates a hierarchy of processes which is used to calculate prime numbers using different prime-finding algorithms implemented in different executables. The processes hierarchy can also be used as a basis for any kind of concurrent calculation that can be split between different programs.

## Compilation:

By typing `make` on a tty in the source code directory, all the necessary programs will be compiled, creating all the necessary executables. The only requirement is a linux/unix machine running a stable version of the gcc compiler. *The program worked fine on Debian based distros.*

## Usage:

The program can be executed from a cli as `./myprime -l lb -u ub -w NumofChildren`  where:

- myprime: is the *root* executable of the project
- lb: is the first number that will be checked as a prime.  The start of the prime-checking range.
- ub: is the last number that will be checked as a prime. The end of the prime-checking range

- NumofChildren: is the number of child processes the myprime process and each primeManager process will create.

## Brief implementation overview:

### Process hierarchy:

**There are three groups of processes:**

1 **Creator/Root:** The basic executable `myprime` is the root of the process’s hierarchy and father of all internal node processes, which in turn create the leaf node processes.

Apart from creating all the needed internal node processes, it also splits the prime-finding range according to the number of internal nodes so that each internal node is responsible for finding a unique sub range of primes. Finally, it collects the found primes *(and the time taken to find them)* from all his children using pipes.

2 **Managers/Internal nodes:** The internal-node processes `primeManager` are responsible for creating  worker/leaf-node processes which together perform the calculations needed to find the primes in the primeManager's sub-range. They also collect the found primes and needed execution time from the leaf node processes using pipes, and compose them into a sorted list. When the child processes are done calculating, the sorted list of results is send to the root node.

3 **Workers/Leaf nodes:** As implied above, the leaf-node processes implement the algorithms which find the primes in a given sub range *(which is a sub range of the internal’s node sub range)* and send them via pipes to the managers/Internal nodes along with the time the calculation took. These processes can be any of `prime1` `prime2` `prime3` executables, which are selected using round dropping in the internal nodes.

----

The number of children processes the creator and each manager spawns is determined by the `NumofChildren` flag on `myprime`, the depth of the hierarchy is always two.
For example, here is a schematic representation of the process hierarchy for 3 child processes:

![Untitled Diagram (1)](https://user-images.githubusercontent.com/17359348/182947379-9757234d-0843-44a6-888b-f87fe6f3d068.png)


Furthermore, each leaf-node process sends a USR1 signal to the root process *(who of course has the according signal handler)* to inform him that he has finished working.

## Source code files overview:

`myPrime.c`: Implements the already mentioned myPrime/Creator processes. The creation of the child processes is done using the fork() and execlp() system calls. The data from the children processes (prime numbers, execution time for leaf nodes etc) is collected through unique pipes for each child process.

`primeManager.c`: Implements the internal-node processes. The program takes as command line arguments the: *1.* range of numbers which will be split and searched for primes from the leaf-node processes, *2.* number of child processes, *3.* the file descriptor of the pipe used to communicate with the root process, *4.* his child number, used as a serial number to determine which child of the root process he is, *5.* the root/myPrime process id to be passed to the leaf nodes. Also, as mentioned, it creates the leaf-nodes, and composes the results of the child process in a sorted list which, when done, is forwarded to the root process.

`prime1.c` & `prime2.c` & `prime3.c`: Each of these programs implement a prime funding algorithm from faster to slower, from trivial to more sophisticated. The two first algorithms were given from the professor alex delis, and the third is a prime finding implementation of my own.

Regarding everything else, the programs work the same way. They all take the same arguments (prime-finding range, pipe file descriptor, process id of root/myPrime process to send the USR1 signal), and as they find the prime numbers they send them to their respective parent using their unique pipe.

`list.c`/`list.c`: Contains the implementation of a C linked list, customized to fit the needs of maintaining and inserting in sorted order prime numbers and the time needed to find them.

`utilities.c` & `utilities.h`: Some useful functions that help maintaining a clean code base by providing a level of abstraction, and preventing code duplication. More in the source code comments.

### Output format:

All the output is printed from the root/myPrime in the following form:

    OUTPUT (per invocation of program):
	Primes in [lb,ub] are:
	result1 time1 result2 time2 result3 time3 result4 time4 ... resulti  timei...
	Found *n* prime numbers in total
	
	Min Time for Workers : mintime msecs
	Max Time for Workers : maxtime msecs
	Num of USR1 Received : numUSR1rec-by-root/number-of-workers-activated
	Time for W0: W0-time msecs
	Time for W1: W1-time msecs
	Time for W2: W2-time msecs
	Time for W3: W3-time msecs
*---Credits to professor alex delis for the above output format ([link](https://www.alexdelis.eu/k22/formatted-output.f20-prj2-v1.txt))---*

 


### Useful notes:

- All of the `prime1,2,3` and `primeManager` executables need to accept arguments, but apart from a check that the correct number of arguments were given, no further checks or error handlers are programed since we assume that the arguments are always well given from the parent process *(and not from the user)*.
- The inter-process communication is done in a non-blocking manner. 
- At `utilities.h` you will find the communication norm *`struct pipeMessage`* used to transfer different types of data through the pipes.

# Salad-Making

A hierarchy of processes that communicate in order to find prime numbers in C

## Summary:

The project uses distinct implemented programs which when executed, simulate the operation of making a salad on behalf of a “restaurant”. There are two types of processes:

1. Chef: this process is responsible for creating the shared memory *("virtual" work bench)* segment and manage the creation of the salads from the Salad-Makers.

2. Salad-Makers: They wait using semaphores until the ingredients they need are available, in order to procced with the creation of the salad. *In our case we use 3 distinct salad makers*

## Compilation:

By typing `make` on a tty in the source code directory, all the necessary programs will be compiled, creating all the necessary executables. The only requirement is a linux/unix machine running a stable version of the gcc compiler. *The program worked fine on Debian based distros.*

## Usage:

To run the project, you must execute **one** chef process and **three** salad makers. This can be done in multiple ways, but the easier is to use multiple terminals. In the first terminal you will run the chef process and in the rest you will run the saladmaker processes OR you can the script saladmakers.sh *(with the shared memory segment ID as argument)* to run all the three saladmaker processes in one terminal with some default arguments.

The chef program can be executed from a cli as `./chef -n numOfSlds -m mantime` where:

- numOfSlds is the total number of salads that need to be prepared.

- mantime is the time interval that the chef rests between consecutive places of ingredients on the table.

Note: the chef will print the shared memory segment ID which you should pass to the salad makers.

The salad maker program can be executed from a cli as `./saladmaker -t1 lb -t2 ub -s shmid ` where:

- t1 is the minimum time interval the saladmaker needs to make a salad. And t2 is the maximum time. In other words, for each salad, the salad maker need a random amount time between t1 and t2 to prepare it.

- Shmid is the unique key (ID) of the shared memory segment, you can find which key to type from the output of the chef program ` Shared memory id: *value*`.

## Brief implementation overview:

Salad making overview / 	Synthesis of concurrent the processes:

Each salad consists of its ingredients, which are: A. one tomato, B. one green paper, C. one small onion. In order to create all the requested salads, there are 4 processes 1x chef process and 3x saladmaker processes.

The chef has baskets at his disposal with all of the three ingredients *(we assume in infinite quantity)*. But each saladmaker has at his disposal only one ingredient basket *(again, in infinite quantity)*, thus each saladmaker awaits to take the two remaining ingredients he needs from the chef. 

Notes:
- The salad making procedure, performed from the salad maker, takes a random amount of time *between t1 and t2*.
- Each time the chef gives an ingredient, he rests for some time.
- We assume that when a salad is ready, the saladmaker gives it to a waiter instantly, in a zero amount of time.
- Each saladmaker waits to be notified only for the "arrival" of the two ingredients he needs, since he already has one ingredient always available.
- The chef only has 2 hands thus (*even though he has all the ingredients at his baskets*) he takes only two  ingredients at a time which are supplied to only one saladmaker, *who is then informed*.

The above schema can be portrayed by the following picture:
![image](https://user-images.githubusercontent.com/17359348/183515842-16dacd56-c0fa-4de2-bbb0-90bb5f6e1297.png)

*Credits to the profesor alex delis for the picture* 

The already mentioned shared memory segment, *which can identify as the work bench*, consists of the following:

- 3 semaphores, one for each saladmaker. These semaphores depict the transfer of ingredients form the chef to the saladmakers, which in conjunction with the fact that each saladmaker is in need for a unique pair of ingredients, gives us the luxury to only use 3 semaphores for each saladmaker and ingredient *instead of 3 ingredient semaphores per saladmaker, which in our case would be a waste*.
- One semaphore that is used from all saladmakers, to inform the chef that there are saladmakers waiting for ingredients.
- One semaphore which is used to protect the critical sections of writing and reading *(the two)* process-shared variables.
- Two process-shared variables, one for the remaining number of salads that must be prepared, and one used as a flag from the saladmakers when they are done working.

At last. While all the programs are executed, they write some diagnostic messages on some log files. The format of which you will find in the end of the documentation.

## Source code files overview:
`chef.c`/`chef.h`: Here you will find the implementation of the chef process. The chef creates the shared memory segment *(and prints it’s id, so that it can be given to all the other saladmaker processes)*. Afterwards, the chef waits for all the saladmakers to be created and get ready to work. Then it gives the ingredients to the saladmakers using the according semaphores until all the salads are made. Finally prints the output of the program before exiting.

`saladMaker.c`/`saladMaker.h`: Implements the saladmaker processes. After reading all the needed command line arguments and perform some valididy checks, it attaches the shared memory segment that the chef has created. Then it informs the chef that he is ready to make salads from his unique semaphore with the chef.

`MySharedMemLib.c`/`MySharedMemLib.h`: Contains functions that create, delete, attach, detach, and in general manage the shared memory segment. This is simple a collection of wrappers for the `sys/shm.h` adjusted to the needs of the project. It helps preserve a simple/clean code base.

`logFileWriter.c`/`logFileWriter.h`: In those files there are all the functions that all the processes use to write their log files. These logs are also accessed from other processes to take some information about them *(ex common execution intervals for the output)*.

Important: Someone could argue that the `mainLog.txt` log file is a common source for all the processes, and therefore should be protected with semaphores. In our specific case this is not needed since each time something is written or read from the files the logWriting functions use fopen() and fclose() when they are done. Thus, any mutual exclusion situations that can lead to deadlock are eliminated, since the implementation of the fopen() & fclose() functions and the operating system will always guarantee that no two *or more* processes can open the same file for writing at the time.

`list.c`/`list.h`: The chef needs to output some common execution intervals. This is a sorted list implementation used to insert those values in a data structure and maintain them in sorted (or the programs needed) order.

Here you will also find the makerTime struct, used to save time intervals.

### Output format:
Stdout of chef:
```
Total #salads: [number] 
#salads of salad_maker1 [pid] : [number]
#salads of salad_maker2 [pid] : [number]
#salads of salad_maker3 [pid] : [number]

Time intervals: (in increasing order)
[start, end] 
[start, end] 
[start, end] 
[start, end] 
...
```
Log files format:
```
[00:00:00:01.00] [2] [Saladmaker1] [Waiting for ingredients] 
[00:00:00:01.00] [3] [Saladmaker2] [Waiting for ingredients] 
[00:00:00:01.00] [4] [Saladmaker3] [Waiting for ingredients] 
[00:00:00:01.10] [1] [Chef] [Selecting ingredients ntomata piperia] 
[00:00:00:01.11] [1] [Chef] [Notify saladmaker #1]
[00:00:00:01.12] [2] [Saladmaker1] [Get ingredients] 
[00:00:00:01.12] [1] [Chef] [Man time for resting]
[00:00:00:01.12] [2] [Saladmaker1] [Start making salad]
[00:00:00:01.30] [1] [Chef] [Selecting ingredients ntomata kremudi] [00:00:00:01.31] [3] [Saladmaker2] [Get ingredients]
[00:00:00:01.32] [2] [Saladmaker1] [End making salad]
...
```

*Credits to the profesor alex delis for the output format ([link]([formatted-output.f20-prj3-v1.pdf (alexdelis.eu)](https://www.alexdelis.eu/k22/formatted-output.f20-prj3-v1.pdf)))* 


### Useful notes:
- `make clean` deletes all the executables and the object files. And `make dclean` removes all the executables and the object files, along with the log files.
- In the stressful environment of the kitchen, the salad makers might request from chef more ingredients than needed salads, since we have 3 saladmakers, In the worst case we might have two more salads than the requested number, which we assume are disposed.
