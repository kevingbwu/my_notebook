# Managing threads

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
            do_something(i);
        }
    }
};

void oops()
{
    int some_local_state=0;
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
    int some_local_state=0;
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
    thread_guard(thread_guard const&)=delete;
    thread_guard& operator=(thread_guard const&)=delete;
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