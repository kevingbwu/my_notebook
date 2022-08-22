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

