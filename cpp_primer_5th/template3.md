# 模板与泛型编程（3）

## 可变参数模板

* 可变参数模板：variadic template
* 可变数目的参数称为参数包：parameter packet
* 模板参数包：template parameter packet, 零个或多个模板参数
* 函数参数包：function parameter packet, 零个或多个函数参数

```c++
// Args is a template parameter pack; rest is a function parameter pack
// Args represents zero or more template type parameters
// rest represents zero or more function parameters
template <typename T, typename... Args>
void foo(const T &t, const Args& ... rest);

int i = 0; double d = 3.14; string s = "how now brown cow";
foo(i, s, 42, d); // three parameters in the pack
foo(s, 42, "hi"); // two parameters in the pack
foo(d, s); // one parameter in the pack
foo("hi"); // empty pack

// 对应的实例化版本
void foo(const int&, const string&, const int&, const
double&);
void foo(const string&, const int&, const char[3]&);
void foo(const double&, const string&);
void foo(const char[3]&);
```

#### sizeof...运算符
```c++
template<typename ... Args> void g(Args ... args) {
    cout << sizeof...(Args) << endl; // number of type parameters
    cout << sizeof...(args) << endl; // number of function parameters
}
```

### 编写可变参数函数模板

**可变参数函数通常是递归的：第一步调用处理包中的第一个实参，然后用剩余实参调用自身**

```c++
// function to end the recursion and print the last element
// this function must be declared before the variadic version of print is defined
template<typename T>
ostream &print(ostream &os, const T &t)
{
    return os << t; // no separator after the last element in the pack
}
// this version of print will be called for all but the last element in the pack
template <typename T, typename... Args>
ostream &print(ostream &os, const T &t, const Args&... rest)
{
    os << t << ", "; // print the first argument
    return print(os, rest...); // recursive call; print the other arguments
}
```

非可变参数模板比可变参数模板更加特例化

### 包扩展

扩展一个包就是将它分解为构成的元素，对每个元素应用模式，获得扩展后的列表。需要提供用于每个扩展元素的模式

```c++
// call debug_rep on each argument in the call to print
template <typename... Args>
ostream &errorMsg(ostream &os, const Args&... rest)
{
    // print(os, debug_rep(a1), debug_rep(a2), ..., debug_rep(an)
    return print(os, debug_rep(rest)...);
}

// 调用导致下面的扩展
errorMsg(cerr, fcnName, code.num(), otherData, "other",
item);
// 扩展
print(cerr, debug_rep(fcnName), debug_rep(code.num()), debug_rep(otherData), debug_rep("otherData"), debug_rep(item));

// passes the pack to debug_rep; print(os, debug_rep(a1, a2, ..., an))
print(os, debug_rep(rest...)); // error: no matching function to call

print(cerr, debug_rep(fcnName, code.num(), otherData, "otherData", item));
```

### 转发参数包

标准库容器的`emplace_back`成员是一个可变参数成员模板, 它用其实参在容器管理的内存空间中直接构造一个元素

* 函数参数定义为模板类型参数的右值引用
* 必须使用forward来保持实参的原始类型

```c++
// fun has zero or more parameters each of which is
// an rvalue reference to a template parameter type
template<typename... Args>
void fun(Args&&... args) // expands Args as a list of rvalue references
{
    // the argument to work expands both Args and args
    work(std::forward<Args>(args)...);
}
```

