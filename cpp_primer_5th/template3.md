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

标准库容器的`emplace_back`成员是一个可变参数成员模板，它用其实参在容器管理的内存空间中直接构造一个元素

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

## 模板特例化

当不能或不希望使用模板版本时，可以定义类或函数模板的一个特例化版本

```c++
// first version; can compare any two types
template <typename T> int compare(const T&, const T&);
// second version to handle string literals
template<size_t N, size_t M>
int compare(const char (&)[N], const char (&)[M]);

const char *p1 = "hi", *p2 = "mom";
compare(p1, p2); // calls the first template
compare("hi", "mom"); // calls the template with two nontype parameters

// 无法将指针转换为数组的引用
```

#### 定义函数模板特例化

```c++
// special version of compare to handle pointers to character arrays
template <>
int compare(const char* const &p1, const char* const &p2)
{
    return strcmp(p1, p2);
}

// T为const char*
// 函数要求一个指向此类型const版本的引用
// const char* const &，一个指向const char的const指针的引用
```
当定义一个特例化版本时，函数参数类型必须与一个先前声明的模板中对应的类型匹配

#### 函数重载与模板特例化

一个特例化版本本质上是一个实例，而非函数名的一个重载版本

> 特例化不影响函数匹配

> 模板及其特例化版本应该声明在同一个头文件中，所有同名模板的声明应该放在前面，然后是这些模板的特例化版本

#### 类模板特例化

为标准库hash模板定义一个特例化版本，用来将Sales_data对象保存在无序容器中

```c++
// open the std namespace so we can specialize std::hash
namespace std {
template <> // we're defining a specialization with
struct hash<Sales_data> // the template parameter of Sales_data
{
    // the type used to hash an unordered container must define these types
    typedef size_t result_type;
    typedef Sales_data argument_type; // by default, this type needs ==
    size_t operator()(const Sales_data& s) const;
    // our class uses synthesized copy control and default constructor
};

size_t
hash<Sales_data>::operator()(const Sales_data& s) const
{
    return hash<string>()(s.bookNo) ^
    hash<unsigned>()(s.units_sold) ^
    hash<double>()(s.revenue);
}}
// close the std namespace; note: no semicolon after the close curly

// uses hash<Sales_data> and Sales_data operator==from § 14.3.1 (p. 561)
unordered_multiset<Sales_data> SDset;

template <class T> class std::hash; // needed for the friend declaration
class Sales_data {
    friend class std::hash<Sales_data>;
    // other members as before
};
```

#### 类模板部分特例化

类模板的部分特例化本身是一个模板，使用它时用户还必须为那些在特例化版本中未指定的模板参数提供实参

可以指定一部分而非所有模板参数，或是参数的一部分而非全部特性

```c++
// original, most general template
template <class T> struct remove_reference {
    typedef T type;
};
// partial specializations that will be used for lvalue and rvalue references
template <class T> struct remove_reference<T&> // lvalue references
{ typedef T type; };
template <class T> struct remove_reference<T&&> // rvalue references
{ typedef T type; };

int i;
// decltype(42) is int, uses the original template
remove_reference<decltype(42)>::type a;
// decltype(i) is int&, uses first (T&) partial specialization
remove_reference<decltype(i)>::type b;
// decltype(std::move(i)) is int&&, uses second (i.e., T&&) partial specialization
remove_reference<decltype(std::move(i))>::type c;
// All three variables, a, b, and c, have type int.
```

#### 特例化成员

可以只特例化特定成员函数而不是特例化整个模板

```c++
template <typename T> struct Foo {
    Foo(const T &t = T()): mem(t) { }
    void Bar() { /* ... */ }
    T mem;
    // other members of Foo
};
template<> // we're specializing a template
void Foo<int>::Bar() // we're specializing the Bar member of Foo<int>
{
    // do whatever specialized processing that applies to ints
}

Foo<string> fs; // instantiates Foo<string>::Foo()
fs.Bar(); // instantiates Foo<string>::Bar()
Foo<int> fi; // instantiates Foo<int>::Foo()
fi.Bar(); // uses our specialization of Foo<int>::Bar()
```