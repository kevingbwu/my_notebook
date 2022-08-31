# Synchronizing concurrent operations

* Waiting for an event
* Waiting for one-off events with futures
* Waiting with a time limit
* Using the synchronization of operations to simplify code

One thread might need to wait for another thread to complete a task before the first thread can complete its own. **In general, it’s common to want a thread to wait for a specific event to happen or a condition to be true.**

Although it would be possible to do this by periodically checking a “task complete” flag or something similar stored in shared data, this is far from ideal. The need to synchronize operations between threads like this is such a common scenario that the C++ Standard Library provides facilities to handle it, in the form of **condition variables** and **futures**.

This chapter will discuss how to wait for events with **condition variables, futures, latches, and barriers**.

## Waiting for an event or other condition

* First option, keep checking a flag in shared data (protected by a mutex) and have the second thread set the flag when it completes the task.

* second option: have the waiting thread sleep for short periods between the checks
```c++
// A second option: have the waiting thread sleep for short periods between the checks
// using the std::this_thread::sleep_for() function
bool flag;
std::mutex m;
void wait_for_flag()
{
    std::unique_lock<std::mutex> lk(m);
    while(!flag)
    {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        lk.lock();
    }
}
```

* The third and preferred option: to use the facilities from the C++ Standard Library to wait for the event itself. **A condition variable is associated with an event or other condition, and one or more threads can wait for that condition to be satisfied. When a thread has determined that the condition is satisfied, it can then notify one or more of the threads waiting on the condition variable in order to wake them up and allow them to continue processing.**

### Waiting for a condition with condition variables

`std::condition_variable`: work with `std::mutex`
`std::condition_variable_any`: work with anything that meets the minimal criteria for being mutex-like, hence the _any suffix

```c++
// Waiting for data to process with std::condition_variable
std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;

void data_preparation_thread()
{
    while(more_data_to_prepare())
    {
        data_chunk const data = prepare_data();
        {
            std::lock_guard<std::mutex> lk(mut);
            data_queue.push(data);
        }
        // call the notify_one() member function on the std::condition_variable instance
        // to notify the waiting thread (if there is one)
        // notify the condition variable after unlocking the mutex
        // so that, if the waiting thread wakes immediately, it doesn’t then have to block again
        data_cond.notify_one();
    }
}
void data_processing_thread()
{
    while(true)
    {
        // processing thread firstlocks the mutex,
        // but this time with a std::unique_lock rather than a std::lock_guard
        std::unique_lock<std::mutex> lk(mut);
        // calls wait() on the std::condition_variable,
        // pass in the lock object and a lambda function that expresses the condition being waited for.
        // If the condition isn’t satisfied (the lambda function returned false),
        // wait() unlocks the mutex and puts the thread in a blocked or waiting state.
        // When the condition variable is notified by a call to notify_one()
        // from the data-preparation thread,
        // the thread wakes from its slumber (unblocks it), reacquires the lock on the mutex, and
        // checks the condition again, returning from wait() with the mutex still locked
        // if the condition has been satisfied.
        data_cond.wait(lk, [] {
            return !data_queue.empty();
        });
        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();
        process(data);
        if(is_last_chunk(data))
            break;
    }
}
```

### Building a thread-safe queue with condition variables

```c++
// Full class definition of a thread-safe queue using condition variables
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue
{
private:
    // locking a mutex is a mutating operation,
    // the mutex object must be marked mutable so it can be locked in empty() and in the copy constructor
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue() {}
    threadsafe_queue(threadsafe_queue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

threadsafe_queue<data_chunk> data_queue;
void data_preparation_thread()
{
    while(more_data_to_prepare())
    {
        data_chunk const data = prepare_data();
        data_queue.push(data);
    }
}

void data_processing_thread()
{
    while(true)
    {
        data_chunk data;
        data_queue.wait_and_pop(data);
        process(data);
        if(is_last_chunk(data))
            break;
    }
}
```

* The call to `notify_one()` will trigger one of the threads currently executing `wait()` to check its condition and return from `wait()`. There’s no guarantee of which thread will be notified or even if there’s a thread waiting to be notified; all the processing threads might still be processing data.
* Several threads are waiting for the same event, and all of them need to respond. The thread preparing the data can call the `notify_all()` member function on the condition variable rather than `notify_one()`. This causes all the threads currently executing wait() to check the condition they’re waiting for.

## Waiting for one-off events with futures

`future`: If a thread needs to wait for a specific one-off event, it somehow obtains a future representing that event

* `std::future<>`: unique futures, an instance of `std::future` is the one and only instance that refers to its associated event
* `std::shared_future<>`: shared futures, multiple instances of `std::shared_future` may refer to the same event

the template parameter is the type of the associated data

### Returning values from background tasks

`std::async`: to start an asynchronous task for which you don’t need the result right away. Rather than giving you a `std::thread` object to wait on, `std::async` returns a `std::future` object, which will eventually hold the return value of the function. When you need the value, you just call `get()` on the future, and the thread blocks until the future is ready and then returns the value.

```c++
// Using std::future to get the return value of an asynchronous task
#include <future>
#include <iostream>

int find_the_answer_to_ltuae();
void do_other_stuff();

int main()
{
    // std::async allows you to pass additional arguments to the function by adding extra
    // arguments to the call, in the same way that std::thread does
    std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
    do_other_stuff();
    std::cout << "The answer is " << the_answer.get() << std::endl;
}
```

```c++
// Passing arguments to a function with std::async

#include <string>
#include <future>

struct X
{
    void foo(int, std::string const&);
    std::string bar(std::string const&);
};

X x;
// Calls p->foo(42,"hello") where p is &x
auto f1 = std::async(&X::foo, &x, 42, "hello");
// Calls tmpx.bar("goodbye") where tmpx is a copy of x
auto f2 = std::async(&X::bar, x, "goodbye");

struct Y
{
    double operator()(double);
};

Y y;
// Calls tmpy(3.141) where tmpy is move-constructed from Y()
auto f3 = std::async(Y(), 3.141);
// Calls y(2.718)
auto f4 = std::async(std::ref(y), 2.718);

X baz(X&);
// Calls baz(x)
std::async(baz, std::ref(x));

class move_only
{
public:
    move_only();
    move_only(move_only&&);
    move_only(move_only const&) = delete;
    move_only& operator=(move_only&&);
    move_only& operator=(move_only const&) = delete;
    void operator()();
};
// Calls tmp() where tmp is constructed from std::move(move_only())
auto f5 = std::async(move_only());
```

**By default, it’s up to the implementation whether `std::async` starts a new thread, or whether the task runs synchronously when the future is waited for.**

`std::launch::deferred`: indicate that the function call is to be deferred until either `wait()` or `get()` is called on the future
`std::launch::async`: indicate that the function must be run on its own thread
`std::launch::deferred | std::launch::async`: indicate that the implementation may choose, this option is the default

```c++
// If the function call is deferred, it may never run

// Run in new thread
auto f6 = std::async(std::launch::async, Y(), 1.2);
// Run in wait() or get()
auto f7 = std::async(std::launch::deferred, baz, std::ref(x));
// Implementation chooses
auto f8 = std::async(std::launch::deferred | std::launch::async, baz, std::ref(x));
auto f9 = std::async(baz, std::ref(x));

// Invoke deferred function
f7.wait();
```

### Associating a task with a future

`std::packaged_task<>`: ties a `future` to a function or callable object. When the `std::packaged_task<>` object is invoked, it calls the associated function or callable object and makes the `future` ready, with the return value stored as the associated data.The template parameter for the `std::packaged_task<>` class template is a function signature, the return type of the specified function signature identifies the type of the `std::future<>` returned from the `get_future()` member function, the argument list of the function signature is used to specify the signature of the packaged task’s function call operator.

```c++
// Partial class definition for a specialization of std::packaged_task<>
template<>
class packaged_task<std::string(std::vector<char>*, int)>
{
public:
    template<typename Callable>
    explicit packaged_task(Callable&& f);
    std::future<std::string> get_future();
    void operator()(std::vector<char>*, int);
};
```

Wrap a task in a `std::packaged_task` and retrieve the future before passing the `std::packaged_task` object elsewhere to be invoked in due course. When you need the result, you can wait for the future to become ready.

```c++
// Running code on a GUI thread using std::packaged_task

#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>

std::mutex m;
std::deque<std::packaged_task<void()>> tasks;
bool gui_shutdown_message_received();
void get_and_process_gui_message();
void gui_thread()
{
    // loops until a message has been received telling the GUI to shut down
    while(!gui_shutdown_message_received())
    {
        // repeatedly polling for GUI messages to handle
        get_and_process_gui_message();
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(m);
            if(tasks.empty())
                continue;
            task = std::move(tasks.front());
            tasks.pop_front();
        }
        task();
    }
}

std::thread gui_bg_thread(gui_thread);
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    std::packaged_task<void()> task(f);
    std::future<void> res = task.get_future();
    std::lock_guard<std::mutex> lk(m);
    tasks.push_back(std::move(task));
    return res;
}
```

### Making (std::)promises

`std::promise<T>`: provides a means of setting a value (of type T) that can later be read through an associated `std::future<T>` object. A `std::promise/std::future` pair would provide one possible mechanism for this facility; the waiting thread could block on the future, while the thread providing the data could use the promise half of the pairing to set the associated value and make the future ready. You can obtain the `std::future` object associated with a given `std::promise` by calling the `get_future()` member function, just like with `std::packaged_task`. When the value of the `promise` is set (using the `set_value()` member function), the future becomes ready and can be used to retrieve the stored value.

```c++
// use a std::promise<bool>/std::future<bool> pair to identify the successful transmission of a block of outgoing data; 
// the value associated with the future is a simple success/failure flag.
// for incoming packets, the data associated with the future is the payload of the data packet.
#include <future>
void process_connections(connection_set& connections)
{
    while(!done(connections))
    {
        for(connection_iterator connection = connections.begin(), end=connections.end();
            connection!=end;
            ++connection)
        {
            if(connection->has_incoming_data())
            {
                data_packet data = connection->incoming();
                std::promise<payload_type>& p = connection->get_promise(data.id);
                p.set_value(data.payload);
            }
            if(connection->has_outgoing_data())
            {
                outgoing_packet data = connection->top_of_outgoing_queue();
                connection->send(data.payload);
                data.promise.set_value(true);
            }
        }
    }
}
```

### Saving an exception for the future

* If the function call invoked as part of `std::async` throws an exception, that exception is stored in the future in place of a stored value, the future becomes ready, and a call to `get()` rethrows that stored exception.
* If you wrap the function in a `std::packaged_task`, when the task is invoked, if the wrapped function throws an exception, that exception is stored in the future in place of the result, ready to be thrown on a call to `get()`.
* `std::promise` provides the same facility, with an explicit function call. If you wish to store an exception rather than a value, you call the `set_exception()` member function rather than `set_value()`.

```c++
extern std::promise<double> some_promise;
try {
    some_promise.set_value(calculate_value());
}
catch(...) {
    some_promise.set_exception(std::current_exception());
    // some_promise.set_exception(std::make_exception_ptr(std::logic_error("foo ")));
}
```

### Waiting from multiple threads

* `std::future` is only moveable (so ownership can be transferred between instances, but only one instance refers to a particular asynchronous result at a time)
* `std::shared_future` instances are copyable (so you can have multiple objects referring to the same associated state).

## Waiting with a time limit

### Clocks

a clock is a class that provides four distinct pieces of information:
* The time now
* The type of the value used to represent the times obtained from the clock
* The tick period of the clock
* Whether or not the clock ticks at a uniform rate and is therefore considered to be a steady clock

* `std::chrono::system_clock`: wall clock time from the system-wide realtime clock
* `std::chrono::steady_clock`: monotonic clock that will never be adjusted
* `std::chrono::high_resolution_clock`: the clock with the shortest tick period available

### Durations

`std::chrono::duration<>` class template: The first template parameter is the type of the representation (such as int, long, or double), and the second is a fraction specifying how many seconds each unit of the duration represents.

```c++
// a number of minutes stored in a short
std::chrono::duration<short, std::ratio<60, 1>>

// a count of milliseconds stored in a double
std::chrono::duration<double, std::ratio<1, 1000>>

// wait for up to 35 milliseconds for a future to be ready:
std::future<int> f = std::async(some_task);
if(f.wait_for(std::chrono::milliseconds(35)) == std::future_status::ready)
    do_something_with(f.get());
```

### Time points

`std::chrono::time_point<>` class template: The time point for a clock, first template parameter specifies which clock it refers to and second template parameter is the units of measurement (a specialization of `std::chrono::duration<>`). The value of a time point is the length of time (in multiples of the specified duration) since a specific point in time called the epoch of the clock.

```c++
auto start = std::chrono::high_resolution_clock::now();
do_something();
auto stop = std::chrono::high_resolution_clock::now();
std::cout << "do_something() took "
          << std::chrono::duration<double, std::chrono::seconds>(stop - start).count()
          << " seconds" << std::endl;
```

```c++
// Waiting for a condition variable with a timeout
#include <condition_variable>
#include <mutex>
#include <chrono>

std::condition_variable cv;
bool done;
std::mutex m;
bool wait_loop()
{
    auto const timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    std::unique_lock<std::mutex> lk(m);
    while(!done)
    {
        if(cv.wait_until(lk, timeout) == std::cv_status::timeout)
            break;
    }
    return done;
}
```