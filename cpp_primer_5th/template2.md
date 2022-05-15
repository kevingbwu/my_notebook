# 模板与泛型编程

## 模板实参推断

从函数实参来确定模板实参的过程被称为**模板实参推断**

### 类型转换与模板类型参数

编译器通常不是对实参进行类型转换，而是生成一个新的模板实例

能够自动应用的类型转换只有：

* const转换
* 数组或函数指针转换

```c++
template <typename T> T fobj(T, T); // arguments are copied
template <typename T> T fref(const T&, const T&); // references
string s1("a value");
const string s2("another value");
fobj(s1, s2); // calls fobj(string, string); const is ignored
fref(s1, s2); // calls fref(const string&, const string&)
// uses premissible conversion to const on s1
int a[10], b[42];
fobj(a, b); // calls f(int*, int*)
fref(a, b); // error: array types don't match
```

#### 使用相同模板参数类型的函数形参

模板实参推断失败

```c++
long lng;
compare(lng, 1024); // error: cannot instantiate compare(long, int)
```

可以将函数模板定义为两个类型参数

```c++
// argument types can differ but must be compatible
template <typename A, typename B>
int flexibleCompare(const A& v1, const B& v2)
{
    if (v1 < v2) return -1;
    if (v2 < v1) return 1;
    return 0;
}

long lng;
flexibleCompare(lng, 1024); // ok: calls flexibleCompare(long, int)
```

#### 正常类型转换应用于普通函数实参

函数模板中用普通类型定义的参数可以正常进行类型转换

```c++
template <typename T>
ostream &print(ostream &os, const T &obj)
{
    return os << obj;
}

print(cout, 42); // instantiates print(ostream&, int)
ofstream f("output");
print(f, 10); // uses print(ostream&, int); converts f to ostream&
```

### 函数模板显式实参

#### 指定显式模板实参

```c++
// T1 cannot be deduced: it doesn't appear in the function parameter list
template <typename T1, typename T2, typename T3>
T1 sum(T2, T3);
```

显式模板实参在尖括号中给出，位于函数名之后，实参列表之前

```c++
// T1 is explicitly specified; T2 and T3 are inferred from the argument types
auto val3 = sum<long long>(i, lng); // long long sum(int, long)
```

显式模板实参按从左至右的顺序与对应的模板参数匹配

```c++
// poor design: users must explicitly specify all three template parameters
template <typename T1, typename T2, typename T3>
T3 alternative_sum(T2, T1);

// error: can't infer initial template parameters
auto val3 = alternative_sum<long long>(i, lng);
// ok: all three parameters are explicitly specified
auto val2 = alternative_sum<long long, int, long>(i, lng);
```

#### 正常类型转换应用于显式指定的实参

对于模板参数已经显式指定了的函数实参，可以进行正常的类型转换

```c++
long lng;
compare(lng, 1024); // error: template parameters don't match
compare<long>(lng, 1024); // ok: instantiates compare(long, long)
compare<int>(lng, 1024); // ok: instantiates compare(int, int)
```

### 尾置返回类型与类型转换

在编译器遇到函数的参数列表之前，并不知道返回结果的准确类型

```c++
template <typename It>
??? &fcn(It beg, It end)
{
    // process the range
    return *beg; // return a reference to an element from the range
}

vector<int> vi = {1,2,3,4,5};
Blob<string> ca = { "hi", "bye" };
auto &i = fcn(vi.begin(), vi.end()); // fcn should return int&
auto &s = fcn(ca.begin(), ca.end()); // fcn should return string&
```

使用尾置返回类型

```c++
// a trailing return lets us declare the return type after the parameter list is seen
template <typename It>
auto fcn(It beg, It end) -> decltype(*beg)
{
    // process the range
    return *beg; // return a reference to an element from the range
}
```

> 迭代器解引用返回左值，decltype推断的类型为元素类型的引用

#### 进行类型转换的标准库模板类

编写类似fcn的函数，但是返回一个元素的值而非引用？

使用标准库的类型转换模板，位头文件`type_traits`

`remove_reference`模板，有一个类型参数和一个名为type的类型成员

* `remove_reference<int&>`, `type`成员为 `int`
* `remove_reference<string&>`, `type` 成员为 `string`
* `remove_reference<decltype(*beg)>::type` 将获得 `beg` 引用的元素类型

```c++
// must use typename to use a type member of a template parameter
template <typename It>
auto fcn2(It beg, It end) ->
typename remove_reference<decltype(*beg)>::type
{
    // process the range
    return *beg; // return a copy of an element from the range
}
```

### 函数指针和实参推断

用函数模板初始化一个函数指针或为一个函数指针赋值，编译器使用函数指针的类型来推断模板实参

```c++
template <typename T> int compare(const T&, const T&);
// pf1 points to the instantiation int compare(const int&, const int&)
int (*pf1)(const int&, const int&) = compare;
```

如果不能从函数指针类型确定模板实参，则会产生编译错误

```c++
// overloaded versions of func; each takes a different function pointer type
void func(int(*)(const string&, const string&));
void func(int(*)(const int&, const int&));
func(compare); // error: which instantiation of compare?
```

可以通过显示实例化来消除歧义

```c++
// ok: explicitly specify which version of compare to instantiate
func(compare<int>); // passing compare(const int&, const int&)
```

### 模板实参推断和引用

#### 从左值引用函数参数推断类型

```c++
template <typename T> void f1(T&); // 实参必须是一个左值
// calls to f1 use the referred-to type of the argument as the template parameter type
f1(i); // i is an int; template parameter T is int
f1(ci); // ci is a const int; template parameter T is const int
f1(5); // error: argument to a & parameter must be an lvalue
```

```c++
template <typename T> void f2(const T&); // 可以接受一个右值
// parameter in f2 is const &; const in the argument is irrelevant
// in each of these three calls, f2's function parameter is inferred as const int&
f2(i); // i is an int; template parameter T is int
f2(ci); // ci is a const int, but template parameter T is int
f2(5); // a const & parameter can be bound to an rvalue; T is int
```

#### 从右值引用函数参数推断类型

```c++
template <typename T> void f3(T&&);
f3(42); // argument is an rvalue of type int; template parameter T is int
```

#### 引用折叠和右值引用参数

通常不能将一个右值引用绑定到一个左值上

* 例外1：将左值引用传递给函数的右值引用参数，且此右值引用指向模板类型参数时，编译器推断模板类型参数为实参的左值引用类型

* 例外2：如果间接创建一个引用的引用，则这些引用形成了**折叠**

> C++11: 右值引用的右值引用，折叠为右值引用

```c++
// X& &, X& &&, and X&& & all collapse to type X&
// The type X&& && collapses to X&&

f3(i); // argument is an lvalue; template parameter T is int&
f3(ci); // argument is an lvalue; template parameter T is const int&

// f3(i)的实例化结果：
// invalid code, for illustration purposes only
void f3<int&>(int& &&); // when T is int&, function parameter is int& &&

// f3的函数参数是T&&且T是int&, 因此T&&是int& &&, 折叠为int&
//实例化f3:
void f3<int&>(int&); // when T is int&, function parameter collapses to int&
```

> 可以将任意类型的实参传递给T&&类型的函数参数

#### 编写接受右值引用参数的模板函数

```c++
template <typename T> void f3(T&& val)
{
    T t = val; // copy or binding a reference?
    t = fcn(t); // does the assignment change only t or val and t?
    if (val == t) { /* ... */ } // always true if T is a reference type
}
```

实际中，右值引用通常应用于两种情况：**模板转发其实参**或**模板被重载**

```c++
template <typename T> void f(T&&); // binds to nonconst rvalues
template <typename T> void f(const T&); // lvalues and const rvalues
```

### 理解 std::move

标准库move函数是使用右值引用的模板的一个例子

#### std::move是如何定义的

```c++
template <typename T>
typename remove_reference<T>::type&& move(T&& t)
{
    return static_cast<typename remove_reference<T>::type&&>(t);
}

string s1("hi!"), s2;
s2 = std::move(string("bye!")); // ok: moving from an rvalue
s2 = std::move(s1); // ok: but after the assigment s1 has indeterminate value
```

#### std::move是如何工作的

在 `std::move(string("bye!"))` 中

* 当向一个右值引用函数参数传递一个右值时，由实参推断出的类型为被引用的类型
* The deduced type of T is string.
* Therefore, remove_reference is instantiated with string.
* The type member of remove_reference<string> is string.
* The return type of move is string&&.
* move’s function parameter, t, has type string&&.
* Accordingly, this call instantiates move<string>, which is the function `string&& move(string &&t)`

在 `std::move(s1)` 中

* 传递给move的实参是一个左值
* The deduced type of T is string& (reference to string, not plain string).
* Therefore, remove_reference is instantiated with string&.
* The type member of remove_reference<string&> is string,
* The return type of move is still string&&.
* move’s function parameter, t, instantiates as string& &&, which collapses to string&.
* Thus, this call instantiates move<string&>, which is
`string&& move(string &t)`

#### 从一个左值static_cast到一个右值引用是允许的

> C++11: 针对右值引用的特例：不能隐式地将一个左值转换为右值引用，可以用static_cast显示地将一个左值转换为一个右值引用

### 转发

某些函数需要将其一个或多个实参连同类型不变地转发给其他函数，需要保存被转发实参的所有性质，包括实参类型是否是const的以及实参是左值还是右值

```c++
// template that takes a callable and two parameters
// and calls the given callable with the parameters ''flipped''
// flip1 is an incomplete implementation: top-level const and references are lost
template <typename F, typename T1, typename T2>
void flip1(F f, T1 t1, T2 t2)
{
    f(t2, t1);
}

void f(int v1, int &v2) // note v2 is a reference
{
    cout << v1 << " " << ++v2 << endl;
}

int i = 0, j = 0;
f(42, i);
cout << i << endl;  // 输出1, f改变了实参i
flip1(f, j, 42);
cout << j << endl;  // 输出0, 通过flip1调用不会改变j

// flip1实例化为
void flip1(void(*fcn)(int, int&), int t1, int t2);
// j的值被拷贝至t1中, f中的引用参数被绑定到t1, 而非j
```

#### 定义能保持类型信息的函数参数

**将一个函数参数定义为一个指向模板类型参数的右值引用，可以保持其对应实参的所有类型信息**

```c++
template <typename F, typename T1, typename T2>
void flip2(F f, T1 &&t1, T2 &&t2)
{
    f(t2, t1);
}

// flip2解决了一半问题, 它对于接受一个左值引用的函数工作得很好，但不能用于接受右值引用参数的函数
void g(int &&i, int& j)
{
    cout << i << " " << j << endl;
}

flip2(g, i, 42); // error: can't initialize int&& from an lvalue
```

> 函数参数与其他任何变量一样，都是左值表达式

#### 在调用中使用`std::forward`保持类型信息

> C++11 `std::forward`返回显示实参类型的右值引用，即`std::forward<T>`的返回类型是`T&&`

```c++
template <typename F, typename T1, typename T2>
void flip(F f, T1 &&t1, T2 &&t2)
{
    f(std::forward<T2>(t2), std::forward<T1>(t1));
}

flip(g, i, 42);  // i将以int&类型传递给g, 42将以int&&类型传递给g
```

### 重载与模板

函数模板可以被另一个模板或一个普通非模板函数重载

#### 编写重载模板

```c++
// 一组调试函数

// print any type we don't otherwise handle
template <typename T> string debug_rep(const T &t)
{
    ostringstream ret; // see § 8.3 (p. 321)
    ret << t; // uses T's output operator to print a representation of t
    return ret.str(); // return a copy of the string to which ret is bound
}

// 定义打印指针的debug_rep版本

// print pointers as their pointer value, followed by the object to which the pointer points
// 此函数不能用于打印字符指针
template <typename T> string debug_rep(T *p)
{
    ostringstream ret;
    ret << "pointer: " << p; // print the pointer's own value
    if (p)
        ret << " " << debug_rep(*p); // print the value to which p points
    else
        ret << " null pointer"; // or indicate that the p is null
    return ret.str(); // return a copy of the string to which ret is bound
}

// 只实例化第一个版本
string s("hi");
cout << debug_rep(s) << endl;

// 两个函数都生成可行的实例
// 第一个版本实例化 debug_rep(const string* &), T绑定到 string *
// 第二个版本实例化 debug_rep(string*), T被绑定到 string
cout << debug_rep(&s) << endl;

// 第二个版本的debug_rep的实例是此调用的精确匹配
```

#### 多个可行的版本

```c++
const string *sp = &s;
cout << debug_rep(sp) << endl;

// 两个版本都是可行的，都是精确匹配
// 选择更特例化的版本
// 模板debug_rep(const T&)本质上可以用于任何类型, 包括指针类型. 此模板比debug_rep(T *)更通用, 后者只能用于指针类型
```

#### 非模板和模板重载

```c++
// print strings inside double quotes
string debug_rep(const string &s)
{
    return '"' + s + '"';
}

// 有两个同样好的可行函数
// debug_rep<string>(const string&)
// debug_rep(const string&)
string s("hi");
cout << debug_rep(s) << endl;

// 编译器选择非模板版本
```

#### 重载模板和类型转换

```c++
cout << debug_rep("hi world!") << endl; // calls debug_rep(T*)

// 三个debug_rep版本都是可行的
// debug_rep(const T&), with T bound to char[10]
// debug_rep(T*), with T bound to const char
// debug_rep(const string&), which requires a conversion from const char* to string. 需要进行类型转换, 没有精确匹配那么好

// 编译器选择 debug_rep(T*), 更加特例化

// 如果希望将字符指针按string处理, 可以定义另外两个非模板重载版本
// convert the character pointers to string and call the string version of debug_rep
string debug_rep(char *p)
{
    return debug_rep(string(p));
}
string debug_rep(const char *p)
{
    return debug_rep(string(p));
}
```

#### 缺少声明可能导致程序行为异常

```c++
template <typename T> string debug_rep(const T &t);
template <typename T> string debug_rep(T *p);
// the following declaration must be in scope
// for the definition of debug_rep(char*) to do the right thing
string debug_rep(const string &);
string debug_rep(char *p)
{
    // if the declaration for the version that takes a const string& is not in scope
    // the return will call debug_rep(const T&) with T instantiated to string
    return debug_rep(string(p));
}
```