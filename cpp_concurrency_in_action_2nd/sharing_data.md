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

### Further guidelines for avoiding deadlock

You can create deadlock with two threads and **no locks** by having each thread call join() on the std::thread object for the other.

**The guidelines for avoiding deadlock all boil down to one idea: don’t wait for another thread if there’s a chance it’s waiting for you.**

```c++
// Using a lock hierarchy to prevent deadlock

// if you hold a lock on a hierarchical_mutex,
// then you can only acquire a lock on another hierarchical_mutex with a lower hierarchy number
hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);
hierarchical_mutex other_mutex(6000);
int do_low_level_stuff();
int low_level_func()
{
    std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
    return do_low_level_stuff();
}
void high_level_stuff(int some_param);
void high_level_func()
{
    std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
    high_level_stuff(low_level_func());
}
void thread_a()
{
    high_level_func();
}

void do_other_stuff();
void other_stuff()
{
    high_level_func();
    do_other_stuff();
}
void thread_b()
{
    std::lock_guard<hierarchical_mutex> lk(other_mutex);
    other_stuff();
}

// A simple hierarchical mutex
class hierarchical_mutex
{
    std::mutex internal_mutex;
    unsigned long const hierarchy_value;
    unsigned long previous_hierarchy_value;
    static thread_local unsigned long this_thread_hierarchy_value;
    void check_for_hierarchy_violation()
    {
        if(this_thread_hierarchy_value <= hierarchy_value)
        {
            throw std::logic_error(“mutex hierarchy violated”);
        }
    }
    void update_hierarchy_value()
    {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }
public:
    explicit hierarchical_mutex(unsigned long value):
        hierarchy_value(value),
        previous_hierarchy_value(0)
    {}
    void lock()
    {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }
    void unlock()
    {
        if(this_thread_hierarchy_value != hierarchy_value)
            throw std::logic_error(“mutex hierarchy violated”);
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }
    bool try_lock()
    {
        check_for_hierarchy_violation();
        if(!internal_mutex.try_lock())
            return false;
        update_hierarchy_value();
        return true;
    }
};

// When thread_local is applied to a variable of block scope
// the storage-class-specifier static is implied if it does not appear explicitly
thread_local unsigned long
hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);
```

### Flexible locking with std::unique_lock

`std::unique_lock` provides a bit more flexibility than `std::lock_guard` by relaxing the invariants; an `std::unique_lock` instance doesn’t always own the mutex that it’s associated with.

`std::unique_lock`:
* You can pass `std::adopt_lock` as a second argument to the constructor to have the lock object manage the lock on a mutex
* You can also pass `std::defer_lock` as the second argument to indicate that the mutex should remain unlocked on construction. The lock can then be acquired later by calling `lock()` on the `std::unique_lock` object (not the mutex) or by passing the `std:: unique_lock` object to `std::lock()`.

```c++
class some_big_object;
void swap(some_big_object& lhs,some_big_object& rhs);

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
        std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
        std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
        std::lock(lock_a, lock_b);
        swap(lhs.some_detail, rhs.some_detail);
    }
};
```

Unless you’re going to be transferring lock ownership around or doing something else that requires `std::unique_lock`, you’re still better off using the C++17 variadic `std::scoped_lock` if it’s available to you.

when to use `std::unique_lock`:
* deferred locking
* where the ownership of the lock needs to be transferred from one scope to another

### Transferring mutex ownership between scopes

One possible use is to allow a function to lock a mutex and transfer ownership of that lock to the caller, so the caller can then perform additional actions under the protection of the same lock.

```c++
// get_lock() function locks the mutex and then prepares the data before returning the lock to the caller
std::unique_lock<std::mutex> get_lock()
{
    extern std::mutex some_mutex;
    std::unique_lock<std::mutex> lk(some_mutex);
    prepare_data();
    return lk;
}
void process_data()
{
    std::unique_lock<std::mutex> lk(get_lock());
    do_something();
}
```

### Locking at an appropriate granularity

`std::unique_lock` works well in this situation, because you can call `unlock()` when the code no longer needs access to the shared data and then call `lock()` again if access is required later in the code.

```c++
void get_and_process_data()
{
    std::unique_lock<std::mutex> my_lock(the_mutex);
    some_class data_to_process = get_next_data_chunk();
    my_lock.unlock();   // Don’t need mutex locked across call to process()
    result_type result = process(data_to_process);
    my_lock.lock();     // Relock mutex to write result
    write_result(data_to_process, result);
}
```

```c++
// Locking one mutex at a time in a comparison operator
class Y
{
private:
    int some_detail;
    mutable std::mutex m;
    int get_detail() const
    {
        std::lock_guard<std::mutex> lock_a(m);
        return some_detail;
    }
public:
    Y(int sd) : some_detail(sd) {}
    friend bool operator==(Y const& lhs, Y const& rhs)
    {
        if(&lhs == &rhs)
            return true;
        int const lhs_value = lhs.get_detail();
        int const rhs_value = rhs.get_detail();
        return lhs_value == rhs_value;
    }
};
```

## Alternative facilities for protecting shared data