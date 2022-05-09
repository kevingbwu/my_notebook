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