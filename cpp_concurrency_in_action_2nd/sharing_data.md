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

## Protecting shared data with mutexes

mutex (mutual exclusion)

Before accessing a shared data structure, you lock the mutex associated with that data, and when you’ve finished accessing the data structure, you unlock the mutex.

The Thread Library then ensures that once one thread has locked a specific mutex, all other threads that try to lock the same mutex have to wait until the thread that successfully locked the mutex unlocks it.

### Using mutexes in C++

`std::mutex`
* lock() member function
* unlock() member function

`std::lock_guard` class template: implements that RAII idiom for a mutex; it locks the supplied mutex on construction and unlocks it on destruction, ensuring a locked mutex is always correctly unlocked

```c++
// Protecting a list with a mutex
#include <list>
#include <mutex>
#include <algorithm>

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int new_value)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    return std::find(some_list.begin(),some_list.end(),value_to_find) != some_list.end();
}

// C++17
std::lock_guard guard(some_mutex);  // class template argument deduction
std::scoped_lock guard(some_mutex); // C++17 also introduces an enhanced version of lock guard called std::scoped_lock
```

### Structuring code for protecting shared data -- To protect the right data

```c++
// Accidentally passing out a reference to protected data
class some_data
{
    int a;
    std::string b;
public:
    void do_something();
};

class data_wrapper
{
private:
    some_data data;
    std::mutex m;
public:
    template<typename Function>
    void process_data(Function func)
    {
        std::lock_guard<std::mutex> l(m);
        func(data);
    }
};

some_data* unprotected;
void malicious_function(some_data& protected_data)
{
    unprotected = &protected_data;
}

data_wrapper x;
void foo()
{
    x.process_data(malicious_function);
    unprotected->do_something();
}
```

**Don’t pass pointers and references to protected data outside the scope of the lock, whether by returning them from a function, storing them in externally visible memory, or passing them as arguments to user-supplied functions.**

### Spotting race conditions inherent in interfaces

```c++
stack<int> s;
if(!s.empty())
{
    int const value = s.top();
    s.pop();
    do_something(value);
}

// With a shared stack object, this call sequence is no longer
// safe, because there might be a call to pop() from another thread that removes the last
// element in between the call to empty() and the call to top()

// This is therefore a classic race condition,
// and the use of a mutex internally to protect the stack contents
// doesn’t prevent it; it’s a consequence of the interface.
```

```c++
// the class definition for a stack with no race conditions in the interface
// The pop() functions throw an empty_stack exception if the stack is empty

#include <exception>
#include <memory>

struct empty_stack: std::exception
{
    const char* what() const noexcept;
};

template<typename T>
class threadsafe_stack
{
public:
    threadsafe_stack();
    threadsafe_stack(const threadsafe_stack&);
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value);
    std::shared_ptr<T> pop();
    void pop(T& value);
    bool empty() const;
};

// This simplification of the interface allows for better control over the data;
// you can ensure that the mutex is locked for the entirety of an operation
```

```c++
// A fleshed-out class definition for a thread-safe stack
#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack: std::exception
{
    const char* what() const throw();   // throw() is an exception specification 
                                        // which prevents it from throwing any exceptions.
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
```

### Deadlock: the problem and a solution

Deadlock: Each of a pair of threads needs to lock both of a pair of mutexes to perform some operation, and each thread has one mutex and is waiting for the other. Neither thread can proceed, because each is waiting for the other to release its mutex. **It’s the biggest problem with having to lock two or more mutexes in order to perform an operation**.

The common advice for avoiding deadlock is to always lock the two mutexes in the same order: if you always lock mutex A before mutex B, then you’ll never deadlock.

`std::lock`: a function that can lock two or more mutexes at once without risk of deadlock.

```c++
// Using std::lock() and std::lock_guard in a swap operation
class some_big_object;
void swap(some_big_object& lhs, some_big_object& rhs);

class X
{
private:
    some_big_object some_detail;
    std::mutex m;
public:
    X(some_big_object const& sd) : some_detail(sd) {}
    friend void swap(X& lhs, X& rhs)
    {
        if(&lhs == &rhs)
            return;
        std::lock(lhs.m, rhs.m);    // the call to std::lock() locks the two mutexes
        // std::adopt_lock parameter is supplied in addition to the mutex to indicate to the std::lock_guard objects 
        // that the mutexes are already locked, and they should adopt the ownership of the existing lock on the mutex
        // rather than attempt to lock the mutex in the constructor
        std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
        std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
        swap(lhs.some_detail, rhs.some_detail);
    }
};
```

`std::scoped_lock<>`: C++17, this is exactly equivalent to `std::lock_guard<>`, except that it is a variadic template, accepting a list of mutex types as template parameters, and a list of mutexes as constructor arguments.

```c++
// The mutexes supplied to the constructor are locked using the same algorithm as std::lock, 
// so that when the constructor completes they are all locked,
// and they are then all unlocked in the destructor
void swap(X& lhs, X& rhs)
{
    if(&lhs == &rhs)
        return;
    // C++17: automatic deduction of class template parameters
    // std::scoped_lock<std::mutex, std::mutex> guard(lhs.m, rhs.m);
    std::scoped_lock guard(lhs.m, rhs.m);
    swap(lhs.some_detail, rhs.some_detail);
}
```

Although `std::lock` (and `std::scoped_lock<>`) can help you avoid deadlock in those cases where you need to acquire two or more locks together, **it doesn’t help if they’re acquired separately**.