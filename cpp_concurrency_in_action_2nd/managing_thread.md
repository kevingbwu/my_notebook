# Managing threads

* Starting threads, and various ways of specifying code to run on a new thread
* Waiting for a thread to finish versus leaving it to run
* Uniquely identifying threads

## Basic thread management

### Launching a thread

Starting a thread using the C++ Standard Library always boils down to constructing a std::thread object

```c++
void do_some_work();
std::thread my_thread(do_some_work);

// std::thread works with any callable type
class background_task
{
public:
    void operator()() const
    {
        do_something();
        do_something_else();
    }
};
background_task f;
std::thread my_thread(f);
```

Once you’ve started your thread, you need to explicitly decide whether to wait for it to finish (by joining with it) or leave it to run on its own (by detaching it).

```c++
// A function that returns while a thread still has access to local variables
struct func
{
    int& i;
    func(int& i_) : i(i_) {}
    void operator()()
    {
        for(unsigned j = 0; j < 1000000; ++j)
        {
            do_something(i);  // Potential access to dangling reference
        }
    }
};

void oops()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    my_thread.detach();
}
```

One common way to handle this scenario is to make the thread function self-contained and copy the data into the thread rather than sharing the data.

Alternatively, you can ensure that the thread has completed execution before the function exits by joining with the thread.

### Waiting for a thread to complete

You can call join() **only once** for a given thread; once you’ve called join(), the std::thread object is no longer joinable, and joinable() will return false.

### Waiting in exceptional circumstances

* If you’re detaching a thread, you can usually call detach() immediately after the thread has been started
* The call to join() is liable to be skipped if an exception is thrown after the thread has been started but before the call to join()

```c++
// Waiting for a thread to finish
struct func;
void f()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    try
    {
        do_something_in_current_thread();
    }
    catch(...)
    {
        t.join();
        throw;
    }
    t.join();
}

// Using RAII to wait for a thread to complete
class thread_guard
{
    std::thread& t;
public:
    explicit thread_guard(std::thread& t_) : t(t_) {}
    ~thread_guard()
    {
        if(t.joinable())
        {
            t.join();
        }
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

struct func;
void f()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
    do_something_in_current_thread();
}
```

### Running threads in the background

```c++
// Detaching a thread to handle other documents
void edit_document(std::string const& filename)
{
    open_document_and_display_gui(filename);
    while(!done_editing())
    {
        user_command cmd = get_user_input();
        if(cmd.type==open_new_document)
        {
            std::string const new_name = get_filename_from_user();
            std::thread t(edit_document, new_name);
            t.detach();
        }
        else
        {
            process_user_input(cmd);
        }
    }
}
```

## Passing arguments to a thread function

It’s important to bear in mind that by default, **the arguments are copied into internal storage, where they can be accessed by the newly created thread of execution**, and then passed to the callable object or function as rvalues as if they were temporaries.

```c++
void f(int i, std::string const& s);
// there’s a significant chance that the oops function will exit
// before the buffer has been converted to a std::string on the new thread
// thus leading to undefined behavior
void oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i", some_param);
    std::thread t(f, 3, buffer);
    t.detach();
}

void f(int i,std::string const& s);
// avoids dangling pointer
void not_oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i", some_param);
    std::thread t(f, 3, std::string(buffer));
    t.detach();
}

void update_data_for_widget(widget_id w, widget_data& data);
// fail to compile
// because you can't pass an rvalue to a function that expects a non-const reference.
void oops_again(widget_id w)
{
    widget_data data;
    std::thread t(update_data_for_widget, w, data);
    display_status();
    t.join();
    process_widget_data(data);
}

// wrap the arguments that need to be references in std::ref
std::thread t(update_data_for_widget, w, std::ref(data));
```

Both the operation of the **std::thread** constructor and the operation of **std::bind** are defined in terms of the same mechanism.

```c++
class X
{
public:
    void do_lengthy_work();
};

X my_x;
std::thread t(&X::do_lengthy_work, &my_x);  // this指针作为第一个参数
```

## Transferring ownership of a thread

`std::thread`: movable but not copyable

```c++
void some_function();
void some_other_function();
// a new thread is started and associated with t1
std::thread t1(some_function);
// Ownership is then transferred over to t2 when t2 is constructed
// t1 no longer has an associated thread of execution; the thread running some_function is now associated with t2.
std::thread t2 = std::move(t1);
// a new thread is started and associated with a temporary std::thread object
t1 = std::thread(some_other_function);
// t3 is default-constructed, which means that it’s created without any associated thread of execution
std::thread t3;
// Ownership of the thread currently associated with t2 is transferred into t3
t3 = std::move(t2);
// transfers ownership of the thread running some_function back to t1 where it started
// in this case t1 already had an associated thread (which was running some_other_function),
// so std::terminate() is called to terminate the program!
t1 = std::move(t3);
```

```c++
// Returning a std::thread from a function
std::thread f()
{
    void some_function();
    return std::thread(some_function);
}
std::thread g()
{
    void some_other_function(int);
    std::thread t(some_other_function,42);
    return t;
}

void f(std::thread t);
void g()
{
    void some_function();
    f(std::thread(some_function));
    std::thread t(some_function);
    f(std::move(t));
}
```

```c++
// scoped_thread and example usage
class scoped_thread
{
    std::thread t;
public:
    explicit scoped_thread(std::thread t_) : t(std::move(t_))
    {
        if(!t.joinable())
            throw std::logic_error(“No thread”);
    }
    ~scoped_thread()
    {
        t.join();
    }
    scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

struct func;
void f()
{
    int some_local_state;
    scoped_thread t{std::thread(func(some_local_state))};
    do_something_in_current_thread();
}
```

One of the proposals for C++17 was for a joining_thread class that would be similar to std::thread, except that it would automatically join in the destructor much like scoped_thread does

```c++
// A joining_thread class
class joining_thread
{
    std::thread t;
public:
    joining_thread() noexcept = default;
    template<typename Callable,typename ... Args>
    explicit joining_thread(Callable&& func, Args&& ... args) :
        t(std::forward<Callable>(func),std::forward<Args>(args)...)
    {}
    explicit joining_thread(std::thread t_) noexcept:
        t(std::move(t_))
    {}
    joining_thread(joining_thread&& other) noexcept:
        t(std::move(other.t))
    {}
    joining_thread& operator=(joining_thread&& other) noexcept
    {
        if(joinable())
            join();
        t = std::move(other.t);
        return *this;
    }
    joining_thread& operator=(std::thread other) noexcept
    {
        if(joinable())
            join();
        t = std::move(other);
        return *this;
    }
    ~joining_thread() noexcept
    {
        if(joinable())
            join();
    }
    void swap(joining_thread& other) noexcept
    {
        t.swap(other.t);
    }
    std::thread::id get_id() const noexcept
    {
        return t.get_id();
    }
    bool joinable() const noexcept
    {
        return t.joinable();
    }
    void join()
    {
        t.join();
    }
    void detach()
    {
        t.detach();
    }
    std::thread& as_thread() noexcept
    {
        return t;
    }
    const std::thread& as_thread() const noexcept
    {
        return t;
    }
};
```

```c++
// Spawns some threads and waits for them to finish
void do_work(unsigned id);
void f()
{
    std::vector<std::thread> threads;
    for(unsigned i = 0; i < 20; ++i)
    {
        threads.emplace_back(do_work,i);
    }
    for(auto& entry: threads)
        entry.join();
}
```

## Choosing the number of threads at runtime

`std::thread::hardware_concurrency()`: This function returns an indication of the number of threads that can truly run concurrently for a given execution of a program.

```c++
// a simple implementation of a parallel version of std::accumulate
template<typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T &result) {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads =
            (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads =
            std::thread::hardware_concurrency();
    unsigned long const num_threads =
            std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(
                accumulate_block<Iterator, T>(),
                block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    accumulate_block<Iterator, T>()(
            block_start, last, results[num_threads - 1]);
    for (auto &entry: threads)
        entry.join();
    return std::accumulate(results.begin(), results.end(), init);
}
```

## Identifying threads

`std::thread::id` type

* the identifier for a thread can be obtained from its associated `std::thread` object by calling the `get_id()` member function

* the identifier for the current thread can be obtained by calling `std::this_thread::get_id()`, which is also defined in the <thread> header

```c++
std::thread::id master_thread;
void some_core_part_of_algorithm()
{
    if(std::this_thread::get_id() == master_thread)
    {
        do_master_thread_work();
    }
    do_common_work();
}
```