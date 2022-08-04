# Concurrent-Primes

*This program was written as part of the Operating System course at DIT – UoA. Credits to the professor of the course at the time, Alex Delis.*


## Summary:

This project focuses on creating and coordinating processes utilizing the appropriate linux/unix system calls *(fork(), exec(), wait(), kill(), mkfifo(), pipe() etc)*. By using these system calls, it creates a hierarchy of processes which is used to calculate prime numbers using different prime-finding algorithms implemented in different executables. The processes hierarchy can also be used a basis for any kind of concurrent calculation split between different programs.

## Compilation and execution instructions:

The makefile allows you can compile the program by typing `make` in the source code directory, this will create all the necessary executable files. The only requirement is a linux/unix machine running a stable version of the gcc compiler. *_Personally the program worked fine on Debian based distros.__*_

The program can be executed from a cli as `_./myprime -l lb -u ub -w NumofChildren_ `  where:

- myprime: is the executable of the project

- lb: is the first number that will be checked as a prime. The start of the prime-check range.

- ub: is the last number that will be checked as a prime. The end of the prime-check range

- NumofChildren: is the number of child processes the myprime process and each primeManagger will create.

## Implementation overviewy:

#### Process hierarchy:

**There are three groups of processes:**

1 **Creator:** The basic executable `myprime` is the root of the process’s hierarchy and father of all internal node processes which in turn create the leaf node processes.

Apart from creating all the needed internal node processes, it also splits the range of numbers according to the number of internal nodes so that each internal node is responsible for finding a unique sub range of primes. Finally, it collects the primes of the sub-ranges using pipes.

2 **Managers/Internal nodes:** The internal node processes `primeManager` are responsible for creating a number of worker/leaf node processes which together perform the calculations needed to find the primes in the give sub-range. They also collect the primes and total execution time from the leaf node processes using pipes. The collected information is inserted into a sorted list which when filled is send to the creator.

3 **Workers/Leaf nodes:** As implied above, the leaf node processes implement the algorithms which find the primes in a sub range *(which is a sub range of internal’s node sub range)*, and send them via pipes to the managers/Internal nodes along with the time the calculation took. These processes can be any of `prime1` `prime2` `prime3` executables, which are selected using round dropping in the internal nodes.

The number of children processes is determined by a flag on `myprime`, the depth of the hierarchy is always two.
For example, here is a schematic representation of the process hierarchy for 3 child processes:

![Untitled Diagram (1)](https://user-images.githubusercontent.com/17359348/182947379-9757234d-0843-44a6-888b-f87fe6f3d068.png)


Furthermore, each child process sends a USR1 signal to the parent *(who of course has the according signal handler)*

Useful notes:

- All of the `prime1,2,3` and `primeManager` executables need to accept arguments, but apart from a check that the correct number of arguments were given, no further checks or error handlers are programed since we assume that the arguments are always well given from the parent process and not form the user.

- At `utilities.h` you will find a communication norm used to transfer different types of data through the pipes.
