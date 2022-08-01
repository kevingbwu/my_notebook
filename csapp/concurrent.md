# Concurrent Programming

* Logical control flows are **concurrent** if they overlap in time.

* Applications that are partitioned into concurrent flows often run faster on multi-core machines than on uniprocessor machines because the flows execute in **parallel** rather than being interleaved.

Modern operating systems provide three basic approaches for building concurrent programs:

* Processes
* I/O
* Threads

## Concurrent Programming with Processes

A process is the operating system’s abstraction for a running program.

Processes have a clean model for sharing state information between parents and children: **file tables are shared and user address spaces are not**

The term **Unix IPC** is typically reserved for a hodgepodge of techniques that allow processes to communicate with other processes that are running on the same host. Examples include **pipes, FIFOs, SystemVshared memory, and System V semaphores**.

## Concurrent Programming with I/O Multiplexing

I/O multiplexing: The basic idea is to use the select function to ask the kernel to suspend the process, returning control to the application only after one or more I/O events have occurred.

### A Concurrent Event-Driven Server Based on I/O Multiplexing

**Event-driven programs**: Flows make progress as a result of certain events. The general idea is to model logical flows as state machines. Informally, **a state machine is a collection of states, input events, and transitions that map states and input events to states**. Each transition maps an (input state, input event) pair to an output state. State machines are typically drawn as directed graphs, where nodes represent states, directed arcs represent transitions, and arc labels represent input events. Astate machine begins execution
in some initial state. Each input event triggers a transition from the current state to the next state.

### Pros and Cons of I/O Multiplexing

A significant disadvantage of event-driven designs is **coding complexity**. Another significant disadvantage of event-based designs is that they cannot fully utilize multi-core processors.

## Concurrent Programming with Threads

In modern systems a process can actually consist of multiple execution units, called **threads**, each running in the context of the process and sharing the same code and global data.

Each thread has its own thread context, including a unique integer thread ID (TID), stack, stack pointer, program counter, general-purpose registers, and condition codes.

### Thread Execution Model

Each process begins life as a single thread called the main thread. The main thread is distinguished from other threads only in the sense that it is always the first thread to run in the process.

### Posix Threads

Posix threads (Pthreads) is a standard interface for manipulating threads from C programs. It was adopted in 1995 and is available on all Linux systems. Pthreads defines about 60 functions that allow programs to create, kill, and reap threads, to share data safely with peer threads, and to notify peers about changes in the system state.

When the call to **pthread_create** returns, the main thread and the newly created peer thread are running concurrently, and tid contains the ID of the new thread.

The main thread waits for the peer thread to terminate with the call to **pthread_join**.

At any point in time, a thread is **joinable or detached**.
* A joinable thread can be reaped and killed by other threads. Its memory resources (such as the stack) are not freed until it is reaped by another thread.
* A detached thread cannot be reaped or killed by other threads. Its memory resources are freed automatically by the system when it terminates.

By default, threads are created joinable. In order to avoid memory leaks, each joinable thread should be either explicitly reaped by another thread or detached by a call to the pthread_detach function.

## Shared Variables in Threaded Programs

### Threads Memory Model

Each thread has its own separate **thread context**, which includes a thread ID, stack, stack pointer, program counter, condition codes, and general-purpose register values. Each thread shares the rest of the **process context** with the other threads. This includes the entire user virtual address space, which consists of read-only text (code), read/write data, the heap, and any shared library code and data areas. The threads also share the same set of open files. **Registers are never shared, whereas virtual memory is always shared.**

### Mapping Variables to Memory

* Global variables: At run time, the read/write area of virtual memory contains exactly one instance of each global variable that can be referenced by any thread.
* Local automatic variables: At run time, each thread’s stack contains its own instances of any local automatic variables.
* Local static variables: As with global variables, the read/write area of virtual memory contains exactly one instance of each local static Section 12.5 Synchronizing Threads with Semaphores 1031 variable declared in a program.

### Shared Variables

A variable v is shared if and only if one of its instances is referenced by more than one thread.

## Synchronizing Threads with Semaphores

We want to ensure that **each thread has mutually exclusive access to the shared variable while it is executing the instructions in its critical section**. The phenomenon in general is known as mutual exclusion.

We must somehow synchronize the threads so that they always have a safe trajectory. A classic approach is based on the idea of a semaphore.

Edsger Dijkstra, a pioneer of concurrent programming, proposed a classic solution to the problem of synchronizing different execution threads based on a special type of variable called a **semaphore**.

A semaphore, s, is a global variable with a nonnegative integer value that can only be manipulated by two special operations, called P and V:

* P(s): If s is nonzero, then P decrements s and returns immediately. If s is zero, then suspend the thread until s becomes nonzero and the thread is restarted by a V operation. After restarting, the P operation decrements s and returns control to the caller.
* V(s): The V operation increments s by 1. If there are any threads blocked at a P operation waiting for s to become nonzero, then the V operation restarts exactly one of these threads, which then completes its P operation by decrementing s.

The names P and V come from the Dutch words proberen (to test) and verhogen (to increment).

A semaphore that is used in this way to protect shared variables is called a **binary semaphore** because its value is always 0 or 1. Binary semaphores whose purpose is to provide mutual exclusion are often called **mutexes**. Performing a P operation on a mutex is called locking the mutex. Similarly, performing the V operation is called unlocking the mutex.

## Using Threads for Parallelism

A sequential program is written as a single logical flow.
A concurrent program is written as multiple concurrent flows.
A parallel program is a concurrent program running on multiple processors.

**Synchronization overhead is expensive** and should be avoided if possible. If it cannot be avoided, the overhead should be amortized by as much useful computation as possible.

## Other Concurrency Issues

### Thread Safety

A function is said to be **thread-safe if and only if it will always produce correct results when called repeatedly from multiple concurrent threads**. If a function is not thread-safe, then we say it is thread-unsafe.

* Functions that do not protect shared variables
* Functions that keep state across multiple invocations
* Functions that return a pointer to a static variable
* Functions that call thread-unsafe functions

### Reentrancy

There is an important class of thread-safe functions, known as **reentrant functions**, that are characterized by the property that they do not reference any shared data when they are called by multiple threads.

It is important to realize that reentrancy is sometimes a property of both the caller and the callee, and not just the callee alone.

### Using Existing Library Functions in Threaded Programs

### Races

**A race occurs when the correctness of a program depends on one thread reaching point x in its control flow before another thread reaches point y**. Races usually occur because programmers assume that threads will take some particular trajectory through the execution state space, forgetting the golden rule that threaded programs must work correctly for any feasible trajectory.

### Deadlocks

Semaphores introduce the potential for a nasty kind of run-time error, called deadlock, where a collection of threads is blocked, waiting for a condition that will never be true.

Mutex lock ordering rule: Given a total ordering of all mutexes, a program is deadlock-free if each thread acquires its mutexes in order and releases them in reverse order.