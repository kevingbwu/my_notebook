# 模板与泛型编程

一个模板就是一个创建类或者函数的蓝图或者说公式，档提供足够的信息时，将蓝图转换为特定的类或者函数，这种转换发生在编译时

## 定义模板

### 函数模板

```c++
template <typenamae T>
int compare(const T &v1, const T &v2) {
    if (v1 < v2>) return -1;
    if (v2 < v2>) return 1;
    returo 0;
}
```

关键字template + <模板参数列表（template parameter list）>

#### 实例化模板参数

```c++
// 实例化处 int compare(const int&, const int&)
cout << compare(1, 0) << endl;
// 实例化处 int compare(const vector<int>&, const vector<int>&)
vector<int> vec1{1, 2, 3}, vec2{4, 5, 6};
cout << compare(vec1, vec2) << endl;
```

#### 模板类型参数

type parameter: 可以用来指定返回类型或函数的参数类型，以及在函数体内用于变量声明或类型转换

```c++
template <typename T>
T foo(T *p) {
    T tmp = *p;
    // ...
    return tmp;
}
```

> **类型参数前必须使用关键字class或typename**

#### 非类型模板参数

```c++
template <unsigned N, ungisned M>
int compare(const char (&p1)[N], const char (&p2)[M]) {
    return strcmp(p1, p2);
}

// 实例化出 int compare(const char (&p1)[3], const char (&p2)[4])
compare("hi", "mom");
```

* 非类型参数可以是整型，指针或左值引用
* 整型实参必须是一个常量表达式
* 指针或引用实参必须具有静态的生存期

#### 编写类型无关的代码

* 模板中的函数参数是const的引用，保证函数可以用于不能拷贝的类型
* 函数体中的条件判断仅使用<比较运算符

```c++
// 即使用于指针也正确的compare版本
template <typename T>
int compare(const T &v1, const T &v2) {
    if (less<T>()(v1, v2)) return -1;
    if (less<T>()(v2, v2)) return 1;
    return 0;
}
```

#### 模板编译

遇到模板定义并不生成代码，只有实例化出末班的一个特定版本时编译器才会生成代码

**为了生成一个特例化版本，编译器需要掌握函数模板或类模板成员函数的定义，因此，模板的头文件通常既包含声明也包含定义**

### 类模板

> **编译器不能为类模板推断模板参数类型，必须在模板名后的尖括号中提供用来代替模板参数的模板实参列表**

#### 定义类模板

```c++
template <typename T>
class Blob {
public:
    typedef T value_type;
    typedef typename std::vector<T>::size_type size_type;
    // constructors
    Blob();
    Blob(std::initializer_list<T> il);
    // number of elements in the Blob
    size_type size() const { return data->size(); }
    bool empty() const { return data->empty(); }
    // add and remove elements
    void push_back(const T &t) {data->push_back(t);}
    // move version; see § 13.6.3 (p. 548)
    void push_back(T &&t) { data->push_back(std::move(t)); }
    void pop_back();
    // element access
    T& back();
    T& operator[](size_type i); // defined in § 14.5 (p. 566)
private:
    std::shared_ptr<std::vector<T>> data;
    // throws msg if data[i] isn't valid
    void check(size_type i, const std::string &msg) const;
};

template <typename T>
void Blob<T>::check(size_type i, const std::string &msg) const
{
    if (i >= data->size())
        throw std::out_of_range(msg);
}

template <typename T>
T& Blob<T>::back()
{
    check(0, "back on empty Blob");
    return data->back();
}

template <typename T>
T& Blob<T>::operator[](size_type i)
{
    // if i is too big, check will throw, preventing access to a nonexistent element
    check(i, "subscript out of range");
    return (*data)[i];
}

template<typename T>
void Blob<T>::pop_back() {
    check(0, "pop_back on empty Blob");
    data->pop_back();
}

template<typename T>
Blob<T>::Blob(): data(std::make_shared<std::vector<T>>()) {}

template<typename T>
Blob<T>::Blob(std::initializer_list<T> il):
        data(std::make_shared<std::vector<T>>(il)) {}
```

#### 实例化模板

```c++
Blob<int> ia;
Blob<int> ia2;

// 实例化出一个与下面定义等价的类
template <> class Blob<int> {
    typedef typename std::vector<int>::size_type size_type;
    Blob();
    Blob(std::initializer_list<int> il);
    // ...
    int& operator[](size_type i);
private:
    std::shared_ptr<std::vector<int>> data;
    void check(size_type i, const std::string &msg) const;
};
```

#### 类模板的成员函数

* 定义在类模板内部的成员函数被隐式声明为内联函数

* 定义在类模板外部的成员函数必须以关键字template开，后接类模板参数列表

#### 类模板成员函数实例化

> **如果一个成员函数没有被使用，则它不会被实例化**

#### 类模板内简化模板类名的使用

```c++
// BlobPtr throws an exception on attempts to access a nonexistent element
template <typename T>
class BlobPtr {
public:
    BlobPtr(): curr(0) { }
    BlobPtr(Blob<T> &a, size_t sz = 0):
            wptr(a.data), curr(sz) { }
    T& operator*() const
    {
        auto p = check(curr, "dereference past end");
        return (*p)[curr]; // (*p) is the vector to which this object points
    }
    // increment and decrement
    BlobPtr& operator++(); // prefix operators
    BlobPtr& operator--();
private:
    // check returns a shared_ptr to the vector if the check succeeds
    std::shared_ptr<std::vector<T>> check(std::size_t, const std::string&) const;
    // store a weak_ptr, which means the underlying vector might be destroyed
    std::weak_ptr<std::vector<T>> wptr;
    std::size_t curr; // current position within the array
};

// 递增递减成员返回BlobPtr&， 而不是Blob<T>&
```

#### 在类模板外使用类模板名

```c++
// postfix: increment/decrement the object but return the unchanged value
template <typename T>
BlobPtr<T> BlobPtr<T>::operator++(int)
{
    // no check needed here; the call to prefix increment will do the check
    BlobPtr ret = *this; // save the current value
    ++*this; // advance one element; prefix ++ checks the increment
    return ret; // return the saved state
}

// 返回类型位于类的作用域之外，必须指出返回类型是一个实例化的BlobPtr
```

#### 类模板和友元

#### 一对一友好关系

```c++
// forward declarations needed for friend declarations in Blob
template <typename> class BlobPtr;
template <typename> class Blob; // needed for parameters in operator==
template <typename T>
bool operator==(const Blob<T>&, const Blob<T>&);

template <typename T>
class Blob {
    // each instantiation of Blob grants access to the version of
    // BlobPtr and the equality operator instantiated with the same type
    friend class BlobPtr<T>;
    friend bool operator==<T>
            (const Blob<T>&, const Blob<T>&);
    // other members as in § 12.1.1 (p. 456)
};
```

#### 通用和特定的模板友好关系

一个类将另一个模板的每个实例都声明为自己的友元，或者限定特定的实例为友元

```c++
// forward declaration necessary to befriend a specific instantiation of a template
template <typename T> class Pal;

class C {  // C is an ordinary, nontemplate class
    friend class Pal<C>; // Pal instantiated with class C is a friend to C
    // all instances of Pal2 are friends to C;
    // no forward declaration required when we befriend all instantiations
    template <typename T> friend class Pal2;
};

template <typename T> class C2 { // C2 is itself a class template
    // each instantiation of C2 has the same instance of Pal as a friend
    friend class Pal<T>; // a template declaration for Pal must be in scope
    // all instances of Pal2 are friends of each instance of C2, prior declaration not needed
    template <typename X> friend class Pal2;
    // Pal3 is a nontemplate class that is a friend of every instance of C2
    friend class Pal3; // prior declaration for Pal3 not needed
};
```

#### 令模板自己的类型参数成为友元

> C++11: 将模板类型参数声明为友元

```c++
template <typename Type> class Bar {
    friend Type; // grants access to the type used to instantiate Bar
    // ...
};
```

#### 模板类型别名

```c++
typedef Blob<string> StrBlob;
```

```c++
// C++11
template<typename T> using twin = pair<T, T>;
twin<string> authors; // authors is a pair<string, string>
// 少写一次string
```

#### 类模板的static成员