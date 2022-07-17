# Concurrent Programming

* Logical control flows are **concurrent** if they overlap in time.

* Applications that are partitioned into concurrent flows often run faster on multi-core machines than on uniprocessor machines because the flows execute in **parallel** rather than being interleaved.

Modern operating systems provide three basic approaches for building concurrent programs:

* Processes
* I/O
* Threads

## Concurrent Programming with Processes

Processes have a clean model for sharing state information between parents andchildren: **file tables are shared and user address spaces are not**

The term **Unix IPC** is typically reserved for a hodgepodge of techniques that allow processes to communicate with other processes that are running on the same host. Examples include **pipes, FIFOs, SystemVshared memory, and System V semaphores**.