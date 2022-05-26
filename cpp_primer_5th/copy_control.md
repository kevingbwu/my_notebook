# 拷贝控制

类如何控制该类型对象的**拷贝、赋值、移动、销毁**

通过定义五种特殊的成员函数来控制这些操作：

* **拷贝构造函数、移动构造函数**：当用同类型的另一对象初始化本对象时做什么
* **拷贝赋值运算符、移动赋值运算符**：将一个对象赋予同类型的另一个对象时做什么
* **析构函数**：对象销毁时做什么

## 拷贝、赋值与销毁

### 拷贝构造函数

**第一个参数时自身类类型的引用，且任何额外参数都有默认值，则此构造函数是拷贝构造函数**

因为载几种情况下会被隐式地使用，所以通常不应该是explicit的

```c++
class Foo {
public:
    Foo(); // default constructor
    Foo(const Foo&); // copy constructor
    // ...
};
```

#### 合成的拷贝构造函数

如果没有为一个类定义拷贝构造函数，编译器会定义一个

```c++
// 合成的拷贝构造函数等价于
class Sales_data {
public:
// other members and constructors as before
// declaration equivalent to the synthesized copy constructor
Sales_data(const Sales_data&);
private:
    std::string bookNo;
    int units_sold = 0;
    double revenue = 0.0;
};

// equivalent to the copy constructor that would be synthesized for Sales_data
Sales_data::Sales_data(const Sales_data &orig):
    bookNo(orig.bookNo), // uses the string copy constructor
    units_sold(orig.units_sold), // copies orig.units_sold
    revenue(orig.revenue) // copies orig.revenue
    { } // empty body
```

#### 直接初始化和拷贝初始化

* 直接初始化：编译器使用普通的函数匹配来选择与提供的参数最匹配的的构造函数
* 拷贝初始化：编译器将右侧运算对象拷贝到正在创建的对象中，通常使用拷贝构造函数完成

```c++
string dots(10, '.'); // direct initialization
string s(dots); // direct initialization
string s2 = dots; // copy initialization
string null_book = "9-999-99999-9"; // copy initialization
string nines = string(100, '9'); // copy initialization

vector<int> v1(10); // ok: direct initialization
vector<int> v2 = 10; // error: constructor that takes a size is explicit
void f(vector<int>); // f's parameter is copy initialized
f(10); // error: can't use an explicit constructor to copy an argument
f(vector<int>(10)); // ok: directly construct a temporary vector from an int
```

拷贝初始化何时发生：
* 用=定义变量
* 将一个对象作为实参传递给一个非引用类型的形参
* 从一个返回类型为非引用类型的函数返回一个对象
* 用花括号列表初始化一个数组中的元素或一个聚合类中的成员

#### 编译器可能绕过拷贝构造函数进行优化

```c++
string null_book = "9-999-99999-9"; // copy initialization
// 编译器可能会改为
string null_book("9-999-99999-9"); // compiler omits the copy constructor
```

### 拷贝赋值运算符

```c++
Sales_data trans, accum;
trans = accum; // uses the Sales_data copy-assignment operator

class Foo {
public:
    Foo& operator=(const Foo&); // assignment operator
    // ...
};

// equivalent to the synthesized copy-assignment operator
Sales_data&
Sales_data::operator=(const Sales_data &rhs)
{
    bookNo = rhs.bookNo; // calls the string::operator=
    units_sold = rhs.units_sold; // uses the built-in int assignment
    revenue = rhs.revenue; // uses the built-in double assignment
    return *this; // return a reference to this object
}
```

如果类未定义拷贝赋值运算符，编译器会合成一个

### 析构函数

释放对象使用的资源，销毁对象的非static数据成员

没有返回值，没有参数

一个类未定义自己的析构函数时，编译器会合成一个

```c++
class Foo {
public:
    ~Foo(); // destructor
    // ...
};

// 合成的析构函数等价于
class Sales_data {
public:
    // no work to do other than destroying the members, which happens automatically
    ~Sales_data() { }
    // other members as before
};
```

#### 何时调用析构函数

```c++
{ // new scope
    // p and p2 point to dynamically allocated objects
    Sales_data *p = new Sales_data; // p is a built-in pointer
    auto p2 = make_shared<Sales_data>(); // p2 is a shared_ptr
    Sales_data item(*p); // copy constructor copies *p into item
    vector<Sales_data> vec; // local object
    vec.push_back(*p2); // copies the object to which p2 points
    delete p; // destructor called on the object pointed to by p
} // exit local scope; destructor called on item, p2, and vec
// destroying p2 decrements its use count; if the count goes to 0, the object is freed
// destroying vec destroys the elements in vec
```

### 三/五法则

**法则一：一个基本原则时首先确定一个类是否需要定义析构函数，如果需要那么几乎肯定也需要拷贝构造函数和拷贝赋值运算符**

```c++
class HasPtr {
public:
    HasPtr(const std::string &s = std::string()):
        ps(new std::string(s)), i(0) { }
    ~HasPtr() { delete ps; }
    // WRONG: HasPtr needs a copy constructor and copy-assignment operator
    // other members as before
};

HasPtr f(HasPtr hp) // HasPtr passed by value, so it is copied
{
    HasPtr ret = hp; // copies the given HasPtr
    // process ret
    return ret; // ret and hp are destroyed
}

HasPtr p("some values");
f(p); // when f completes, the memory to which p.ps points is freed
HasPtr q(p); // now both p and q point to invalid memory!
```

**如果一个类需要一个拷贝构造函数，几乎可以肯定它也需要一个拷贝赋值运算符，反之亦然**