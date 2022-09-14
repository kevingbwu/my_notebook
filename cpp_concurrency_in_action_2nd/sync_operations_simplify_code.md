## Using synchronization of operations to simplify code

Rather than sharing data directly between threads, each task can be provided with the data it needs, and the result can be disseminated to any other threads that need it through the use of futures

### Functional programming with futures

functional programming (FP): **refers to a style of programming where the result of a function call depends solely on the parameters to that function and doesn’t depend on any external state**

`future` can be passed around between threads to allow the result of one computation to depend on the result of another, without any explicit access to shared data.

#### FP-STYLE QUICKSORT

```c++
//A sequential implementation of Quicksort
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    // splice: Transfers elements from one list to another.
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();
    // Rearranges the elements from the range [first, last),
    // in such a way that all the elements for which pred returns true precede all those for which it returns false.
    // The iterator returned points to the first element of the second group.
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t){ return t < pivot; });
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    auto new_lower(sequential_quick_sort(std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(),new_lower);
    return result;
}
```

#### FP-STYLE PARALLEL QUICKSORT

```c++
// Parallel Quicksort using futures
template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(),
        [&](T const& t) { return t < pivot; });
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    std::future<std::list<T>> new_lower(std::async(&parallel_quick_sort<T>, std::move(lower_part)));
    auto new_higher(parallel_quick_sort(std::move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}
```

```c++
// std::result_of: Deduces the return type of an INVOKE expression at compile time
// type: the return type of the Callable type F if invoked with the arguments A
template<typename F, typename A>
std::future<std::result_of<F(A&&)>::type>
spawn_task(F&& f, A&& a)
{
    typedef std::result_of<F(A&&)>::type result_type;
    std::packaged_task<result_type(A&&)> task(std::move(f)));
    std::future<result_type> res(task.get_future());
    std::thread t(std::move(task), std::move(a));
    t.detach();
    return res;
}
```