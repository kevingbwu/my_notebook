# Sharing data between threads

* Problems with sharing data between threads
* Protecting data with mutexes
* Alternative facilities for protecting shared data

## Problems with sharing data between threads

The problems with sharing data between threads are all due to the consequences of **modifying data**

### Race conditions

In concurrency, a race condition is anything where the outcome depends on the relative ordering of execution of operations on two or more threads.

Problematic race conditions typically occur where **completing an operation requires modification of two or more distinct pieces of data**, such as the two link pointers in the example

### Avoiding problematic race conditions

The simplest option is to wrap your data structure with a protection mechanism to ensure that only the thread performing a modification can see the intermediate states where the invariants are broken. **From the point of view of other threads accessing that data structure, such modifications either haven’t started or have completed**.

Another option is to modify the design of your data structure and its invariants so that modifications are done as a series of indivisible changes, each of which preserves the invariants. This is generally referred to as lock-free programming.

Another way of dealing with race conditions is to handle the updates to the data structure as a transaction, just as updates to a database are done within a transaction. [software transactional memory (STM)]

