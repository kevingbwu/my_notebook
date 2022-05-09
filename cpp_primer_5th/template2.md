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