# 重载运算与类型转换

运算符用于类类型的对象时，可以通过运算符重载重新定义运算符的含义

## 基本概念

重载的运算符是具有特殊名字的函数，关键字operator和其后要定义的运算符号，参数数量与该运算符作用的运算对象数量一样多。对于二元运算符，左侧运算对象传递给第一个参数，右侧运算对象传递给第二个参数

如果运算符函数是成员函数，左侧运算对象绑定到隐式的this指针

不能被重载的运算符：`::` `.*` `.` `?:`

不应该重载逗号、取地址、逻辑与、逻辑或运算符

#### 成员或者非成员

* 赋值、下标、调用、成员访问运算符必须是成员
* 复合赋值运算符一般是成员
* 递增、递减、解引用通常是成员
* 算术、相等性、关系、位运算应该是普通的非成员函数

## 输入和输出运算符

### 重载输出运算符`<<`

```c++
ostream &operator<<(ostream &os, const Sales_data &item)
{
    os << item.isbn() << " " << item.units_sold << " "
    << item.revenue << " " << item.avg_price();
    return os;
}
```

输入输出运算符必须是非成员函数，否则，左侧运算对象将是类的一个对象

```c++
Sales_data data;
data << cout; // if operator<< is a member of Sales_data
```

如果输入输出运算符是类的成员，那也得是istream或ostream的成员，但是这两个类属于标准库，不能给标准库中的类添加成员，所以为类自定义IO必须将其定义为非成员函数，一般声明为友元

### 重载输入运算符`>>`

```c++
istream &operator>>(istream &is, Sales_data &item)
{
    double price; // no need to initialize; we'll read into price before we use it
    is >> item.bookNo >> item.units_sold >> price;
    if (is) // check that the inputs succeeded
        item.revenue = item.units_sold * price;
    else
        item = Sales_data(); // input failed: give the object the default state
    return is;
}
```

**输入运算符必须处理输入可能失败的情况**

## 算术和关系运算符

### 算术运算符

非成员函数，使用复合赋值来定义算术运算符，返回一个副本

```c++
// assumes that both objects refer to the same book
Sales_data operator+(const Sales_data &lhs, const Sales_data &rhs)
{
    Sales_data sum = lhs; // copy data members from lhs into sum
    sum += rhs; // add rhs into sum
    return sum;
}
```

### 相等运算符

```c++
bool operator==(const Sales_data &lhs, const Sales_data &rhs)
{
    return lhs.isbn() == rhs.isbn() &&
           lhs.units_sold == rhs.units_sold &&
           lhs.revenue == rhs.revenue;
}
// 一个应该把工作委托给另一个
bool operator!=(const Sales_data &lhs, const Sales_data &rhs)
{
    return !(lhs == rhs);
}
```

### 关系运算符

operator<

## 赋值运算符

拷贝赋值、移动赋值、其他赋值

```c++
class StrVec {
public:
    StrVec &operator=(std::initializer_list<std::string>);
    // other members as in § 13.5
};

StrVec &StrVec::operator=(initializer_list<string> il)
{
    // alloc_n_copy allocates space and copies elements from the given range
    auto data = alloc_n_copy(il.begin(), il.end());
    free(); // destroy the elements in this object and free the space
    elements = data.first; // update data members to point to the new space
    first_free = cap = data.second;
    return *this;
}

// member binary operator: left-hand operand is bound to the implicit this pointer
// assumes that both objects refer to the same book
Sales_data& Sales_data::operator+=(const Sales_data &rhs)
{
    units_sold += rhs.units_sold;
    revenue += rhs.revenue;
    return *this;
}
```

## 下标运算符

通常以所访问元素的引用作为返回值，最好同时定义下标运算符的常量版本和非常量版本

```c++
class StrVec {
public:
    std::string& operator[](std::size_t n)
    { return elements[n]; }
    const std::string& operator[](std::size_t n) const
    { return elements[n]; }
    // other members as in § 13.5
private:
    std::string *elements; // pointer to the first element in the array
};

// assume svec is a StrVec
const StrVec cvec = svec; // copy elements from svec into cvec
// if svec has any elements, run the string empty function on the first one
if (svec.size() && svec[0].empty()) {
    svec[0] = "zero"; // ok: subscript returns a reference to a string
    cvec[0] = "Zip"; // error: subscripting cvec returns a reference to const
}
```

## 递增和递减运算符

### 前置

```c++
class StrBlobPtr {
public:
    // increment and decrement
    StrBlobPtr& operator++(); // prefix operators
    StrBlobPtr& operator--();
    // other members as before
};

// prefix: return a reference to the incremented/decremented object
StrBlobPtr& StrBlobPtr::operator++()
{
    // if curr already points past the end of the container, can't increment it
    check(curr, "increment past end of StrBlobPtr");
    ++curr; // advance the current state
    return *this;
}

// 如果curr(一个无符号数)已经是0，则--cur将产生一个非常大的正数
StrBlobPtr& StrBlobPtr::operator--()
{
    // if curr is zero, decrementing it will yield an invalid subscript
    --curr; // move the current state back one element
    check(curr, "decrement past begin of StrBlobPtr");
    return *this;
}
```

### 后置

后置版本接受一个额外的int类型的形参

```c++
class StrBlobPtr {
public:
    // increment and decrement
    StrBlobPtr operator++(int); // postfix operators
    StrBlobPtr operator--(int);
    // other members as before
};

// postfix: increment/decrement the object but return the unchanged value
StrBlobPtr StrBlobPtr::operator++(int)
{
    // no check needed here; the call to prefix increment will do the check
    StrBlobPtr ret = *this; // save the current value
    ++*this; // advance one element; prefix ++ checks the increment
    return ret; // return the saved state
}
StrBlobPtr StrBlobPtr::operator--(int)
{
    // no check needed here; the call to prefix decrement will do the check
    StrBlobPtr ret = *this; // save the current value
    --*this; // move backward one element; prefix -- checks the decrement
    return ret; // return the saved state
}
```

## 成员访问运算符

解引用 `*` 和箭头 `->`

```c++
class StrBlobPtr {
public:
    std::string& operator*() const
    {
        auto p = check(curr, "dereference past end");
        return (*p)[curr]; // (*p) is the vector to which this object points
    }
    std::string* operator->() const
    {
        // delegate the real work to the dereference operator
        return & this->operator*();
    }
    // other members as before
};

StrBlob a1 = {"hi", "bye", "now"};
StrBlobPtr p(a1); // p points to the vector inside a1
*p = "okay"; // assigns to the first element in a1
cout << p->size() << endl; // prints 4, the size of the first element in a1
cout << (*p).size() << endl; // equivalent to p->size()
```

## 函数调用运算符

如果类重载了函数调用运算符，则可以像使用函数一样使用该类成员，该类的对象称作函数对象。这样的类同时也能存储状态，比普通函数更加灵活

```c++
struct absInt {
    int operator()(int val) const {
        return val < 0 ? -val : val;
    }
};

int i = -42;
absInt absObj; // object that has a function-call operator
int ui = absObj(i); // passes i to absObj.operator()
```

#### 含有状态的函数对象类

```c++
class PrintString {
public:
    PrintString(ostream &o = cout, char c = ' ') : 
        os(o), sep(c) { }
    void operator()(const string &s) const { 
        os << s << sep;
    }
private:
    ostream &os; // stream on which to write
    char sep; // character to print after each output
};

PrintString printer; // uses the defaults; prints to cout
printer(s); // prints s followed by a space on cout
PrintString errors(cerr, '\n');
errors(s); // prints s followed by a newline on cerr

// 函数对象常常作为泛型算法的实参
for_each(vs.begin(), vs.end(), PrintString(cerr, '\n'));
```

### lambda是函数对象

编译器将lambda翻译成一个未命名类的未命名对象，未命名类中含有一个重载的函数调用运算符

```c++
// sort words by size, but maintain alphabetical order for words of the same size
stable_sort(words.begin(), words.end(), [](const string &a, const string &b){ 
    return a.size() < b.size();
});

// 行为类似于下面这个类的未命名对象
// 默认情况下lambda不能改变它捕获的变量，函数运算符是一个const成员函数
class ShorterString {
public:
    bool operator()(const string &s1, const string &s2) const {
        return s1.size() < s2.size();
    }
};

stable_sort(words.begin(), words.end(), ShorterString());
```

* 引用捕获变量，无须在lambda产生的类中将其存储为数据成员
* 值捕获的变量，lambda产生的类必须为每个值捕获的变量建立对应的数据成员，同时创建构造函数

```c++
// get an iterator to the first element whose size() is >= sz
auto wc = find_if(words.begin(), words.end(), [sz](const string &a) {
    return a.size() >= sz;
});

class SizeComp {
public:
    SizeComp(size_t n): sz(n) { } // parameter for each captured variable
    // call operator with the same return type, parameters, and body as the lambda
    bool operator()(const string &s) const
    { return s.size() >= sz; }
private:
    size_t sz; // a data member for each variable captured by value
};

// get an iterator to the first element whose size() is >= sz
auto wc = find_if(words.begin(), words.end(), SizeComp(sz));
```

**lambda产生的类不含默认的构造函数、赋值运算符、析构函数**

### 标准库定义的函数对象

标准库定义了一组表示算术运算符、关系运算符、逻辑运算符的类

```c++
plus<int> intAdd; // function object that can add two int values
negate<int> intNegate; // function object that can negate an int value
// uses intAdd::operator(int, int) to add 10 and 20
int sum = intAdd(10, 20); // equivalent to sum = 30
sum = intNegate(intAdd(10, 20)); // equivalent to sum = -30
// uses intNegate::operator(int) to generate -10 as the second parameter
// to intAdd::operator(int, int)
sum = intAdd(10, intNegate(10)); // sum = 0
```

#### 在算法中使用标准库函数对象

```c++
// passes a temporary function object that applies the < operator to two strings
sort(svec.begin(), svec.end(), greater<string>());
```

标准库规定其函数对象对指针同样适用。比较两个无关指针将产生未定义行为，使用标准库函数对象时良好定义的

```c++
vector<string *> nameTable; // vector of pointers
// error: the pointers in nameTable are unrelated, so < is undefined
sort(nameTable.begin(), nameTable.end(), [](string *a, string *b) { 
    return a < b; 
});
// ok: library guarantees that less on pointer types is well defined
sort(nameTable.begin(), nameTable.end(), less<string*>());
```

**关联容器使用`less<key_type>`对元素排序，因此可以定义一个指针的set或在map中使用指针作为关键值**

### 可调用对象与function

可调用对象：函数、函数指针、lambda表达式、bind创建的类、重载了函数调用运算符的类

可调用对象有自己的类型，两个不同类型的可调用对象可能共享同一种调用形式，例如 `int(int, int)`

#### 不同类型可能具有相同的调用形式

对于可调用对象共享同一种调用形式的情况，有时希望把他们看成具有相同的类型

```c++
// ordinary function
int add(int i, int j) { return i + j; }
// lambda, which generates an unnamed function-object class
auto mod = [](int i, int j) { return i % j; };
// function-object class
struct div {
    int operator()(int denominator, int divisor) {
        return denominator / divisor;
    }
};

// maps an operator to a pointer to a function taking two ints and returning an int
map<string, int(*)(int,int)> binops;

// ok: add is a pointer to function of the appropriate type
binops.insert({"+", add}); // {"+", add} is a pair § 11.2.3
// mod是一个lambda表达式，有自己的类类型
binops.insert({"%", mod}); // error: mod is not a pointer to function
```

#### 标准库function类型

> C++11：标准库类型function

```c++
function<int(int, int)> f1 = add; // function pointer
function<int(int, int)> f2 = div(); // object of a function-object class
function<int(int, int)> f3 = [](int i, int j) {
    return i * j; 
}; // lambda
cout << f1(4,2) << endl; // prints 6
cout << f2(4,2) << endl; // prints 2
cout << f3(4,2) << endl; // prints 8

// table of callable objects corresponding to each binary operator
// all the callables must take two ints and return an int
// an element can be a function pointer, function object, or lambda
map<string, function<int(int, int)>> binops;

map<string, function<int(int, int)>> binops = {
    {"+", add}, // function pointer
    {"-", std::minus<int>()}, // library function object
    {"/", div()}, // user-defined function object
    {"*", [](int i, int j) { return i * j; }}, // unnamed lambda
    {"%", mod} // named lambda object
};

binops["+"](10, 5); // calls add(10, 5)
binops["-"](10, 5); // uses the call operator of the minus<int> object
binops["/"](10, 5); // uses the call operator of the div object
binops["*"](10, 5); // calls the lambda function object
binops["%"](10, 5); // calls the lambda function object
```

不能直接将重载函数的名字存入function类型的对象中,可以存储函数指针

```c++
int add(int i, int j) { return i + j; }
Sales_data add(const Sales_data&, const Sales_data&);
map<string, function<int(int, int)>> binops;
binops.insert( {"+", add} ); // error: which add?

int (*fp)(int,int) = add; // pointer to the version of add that takes two ints
binops.insert( {"+", fp} ); // ok: fp points to the right version of add
// ok: use a lambda to disambiguate which version of add we want to use
binops.insert( {"+", [](int a, int b) { return add(a, b);} } );
```

## 类型转换运算符

`operator type() const` 不能声明返回类型，形参列表也必须为空，通常是const，必须是成员函数

```c++
class SmallInt {
public:
    SmallInt(int i = 0): val(i)
    {
        if (i < 0 || i > 255)
        throw std::out_of_range("Bad SmallInt value");
    }
    operator int() const { return val; }
private:
    std::size_t val;
};

SmallInt si;
si = 4; // implicitly converts 4 to SmallInt then calls SmallInt::operator=
si + 3; // implicitly converts si to int followed by integer addition

// the double argument is converted to int using the built-in conversion
SmallInt si = 3.14; // calls the SmallInt(int) constructor
// the SmallInt conversion operator converts si to int;
si + 3.14; // that int is converted to double using the built-in conversion

class SmallInt;
operator int(SmallInt&); // error: nonmember
class SmallInt {
public:
    int operator int() const; // error: return type
    operator int(int = 0) const; // error: parameter list
    operator int*() const { return 42; } // error: 42 is not a pointer
};
```

实践中，定义向bool的类型转换比较普遍

#### 显式的类型转换运算符

> C++11：explicit conversion operator

```c++
class SmallInt {
public:
    // the compiler won't automatically apply this conversion
    explicit operator int() const { return val; }
    // other members as before
};

SmallInt si = 3; // ok: the SmallInt constructor is not explicit
si + 3; // error: implicit is conversion required, but operator int is explicit
static_cast<int>(si) + 3; // ok: explicitly request the conversion
```

如果表达式被用作条件，显式的类型转换将被隐式地执行

```c++
while (std::cin >> value) 
```