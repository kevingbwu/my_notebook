# The C++ memory model and operations on atomic types

* The details of the C++ memory model
* The atomic types provided by the C++
* Standard Library
* The operations that are available on those types
* How those operations can be used to provide synchronization between threads

## Memory model basics

### Objects and memory locations

### Objects, memory locations, and concurrency

If two threads access separate memory locations, there’s no problem: everything works fine. On the other hand, if two threads access the same memory location, then you have to be careful. If neither thread is updating the memory location, you’re fine; read-only data doesn’t need protection or synchronization. If either thread is modifying the data, there’s a potential for a race condition.

### Modification orders

## Atomic operations and types in C++

**An atomic operation is an indivisible operation**. You can’t observe such an operation half-done from any thread in the system; it’s either done or not done.

In C++, you need to use an atomic type to get an atomic operation in most cases.

### The standard atomic types

The standard atomic types can be found in the `<atomic>` header.

`std::atomic<T>::is_lock_free()`: Checks whether the atomic operations on all objects of this type are lock-free.
* true: operations on a given type are done directly with atomic instructions
* falase: done by using a lock internal

If the atomic operations themselves use an internal mutex then the hoped-for performance gains will probably not materialize, and you might be better off using the easier-to-get-right mutex-based implementation instead.

C++17, all atomic types have `a static constexpr member variable, X::is_ always_lock_free`, which is true if and only if the atomic type X is lock-free for all supported hardware that the output of the current compilation might run on.

The only type that doesn’t provide an `is_lock_free()` member function is `std::atomic_flag`. This type is a simple Boolean flag, and operations on this type are required to be lock-free.

`std::atomic<T>`: The standard atomic types are not copyable or assignable in the conventional sense, in that they have no copy constructors or copy assignment operators.

atomic operations are divided into three categories:
* Store operations
* Load operations
* Read-modify-write operations