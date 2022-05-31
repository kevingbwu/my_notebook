# 拷贝控制

类如何控制该类型对象的**拷贝、赋值、移动、销毁**

通过定义五种特殊的成员函数来控制这些操作：

* **拷贝构造函数、移动构造函数**：当用同类型的另一对象初始化本对象时做什么
* **拷贝赋值运算符、移动赋值运算符**：将一个对象赋予同类型的另一个对象时做什么
* **析构函数**：对象销毁时做什么

## 拷贝、赋值与销毁

### 拷贝构造函数

**第一个参数时自身类类型的引用，且任何额外参数都有默认值，则此构造函数是拷贝构造函数**

因为在几种情况下会被隐式地使用，所以通常不应该是explicit的

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

### 使用=default

=default显式地要求编译器生成合成的版本

```c++
// 类内用=default，合成的函数将隐式地声明为内联的
class Sales_data {
public:
    // copy control; use defaults
    Sales_data() = default;
    Sales_data(const Sales_data&) = default;
    Sales_data& operator=(const Sales_data &);
    ~Sales_data() = default;
    // other members as before
};

// 如果不希望合成的成员是内联函数，应该只对成员的类外定义使用=default
Sales_data& Sales_data::operator=(const Sales_data&) = default;
```

### 阻止拷贝

如果不定义拷贝控制成员，编译器会生成合成的版本。但是如果拷贝操作没有合理的意义，必须采用某种机制组织拷贝或赋值

#### 定义删除的函数

参数列表后加=delete

```c++
struct NoCopy {
    NoCopy() = default; // use the synthesized default constructor
    NoCopy(const NoCopy&) = delete; // no copy
    NoCopy &operator=(const NoCopy&) = delete; // no assignment
    ~NoCopy() = default; // use the synthesized destructor
    // other members
};
```

#### 析构函数不能是删除的成员

如果析构函数被删除，不允许定义该类型的变量。可以动态分配这种类型的对象，不能释放这些对象

```c++
struct NoDtor {
    NoDtor() = default; // use the synthesized default constructor
    ~NoDtor() = delete; // we can't destroy objects of type NoDtor
};
NoDtor nd; // error: NoDtor destructor is deleted
NoDtor *p = new NoDtor(); // ok: but we can't delete p
delete p; // error: NoDtor destructor is deleted
```

#### 合成的拷贝控制成员可能是删除的

对某些类来说，编译器将这些合成的成员定义为删除的：

如果一个类有数据成员不能默认构造、拷贝、复制、销毁，则对应的成员函数将被定义为删除的

#### private拷贝控制

C++11之前，通过将拷贝构造函数和拷贝赋值运算符声明为private的来阻止拷贝

用户代码不能拷贝这个类型的对象，友元和成员函数可以拷贝对象。为了阻止友元和成员函数进行拷贝，将这些拷贝控制成员声明为private的，但并不定义它们

```c++
class PrivateCopy {
    // no access specifier; following members are private by default; see § 7.2
    // copy control is private and so is inaccessible to ordinary user code
    PrivateCopy(const PrivateCopy&);
    PrivateCopy &operator=(const PrivateCopy&);
    // other members
public:
    PrivateCopy() = default; // use the synthesized default constructor
    ~PrivateCopy(); // users can define objects of this type but not copy them
};
```

## 拷贝控制和资源管理

如果一个类需要定义析构函数，那么几乎肯定也需要定义拷贝构造函数和拷贝赋值运算符。为了定义这些成员，首先必须确定拷贝语义。

* 类的行为像一个值：拷贝时副本和原对象是完全独立的，改变副本不会对原对象有任何影响
* 行为像指针的类，副本和原对象使用相同的底层数据，改变副本会改变原对象

### 行为像值的类

```c++
class HasPtr {
public:
    HasPtr(const std::string &s = std::string()) : 
        ps(new std::string(s)), i(0) { }
    // each HasPtr has its own copy of the string to which ps points
    HasPtr(const HasPtr &p) :
        ps(new std::string(*p.ps)), i(p.i) { }
    HasPtr& operator=(const HasPtr &);
    ~HasPtr() { delete ps; }
private:
    std::string *ps;
    int i;
};

// 赋值运算符通常组合了析构函数和构造函数的操作
// 必须正确处理自赋值
HasPtr& HasPtr::operator=(const HasPtr &rhs)
{
    auto newp = new string(*rhs.ps); // copy the underlying string
    delete ps; // free the old memory
    ps = newp; // copy data from rhs into this object
    i = rhs.i;
    return *this; // return this object
}

// WRONG way to write an assignment operator!
HasPtr& HasPtr::operator=(const HasPtr &rhs)
{
    delete ps; // frees the string to which this object points
    // if rhs and *this are the same object, we're copying from deleted memory!
    ps = new string(*(rhs.ps));
    i = rhs.i;
    return *this;
}
```

### 行为像指针的类

最好使用shared_ptr来管理类中的资源

如果希望直接管理资源，使用引用计数，计数器保存在动态内存中

```c++
// 计数器不能作为HasPtr对象的成员，如果引用计数保存在每个对象中，创建p3时如何正确更新？更新p1中的计数器并将其拷贝到p3中，但如何更新p2中的计数器？
HasPtr p1("Hiya!");
HasPtr p2(p1); // p1 and p2 point to the same string
HasPtr p3(p1); // p1, p2, and p3 all point to the same string
```

```c++
class HasPtr {
public:
    // constructor allocates a new string and a new counter, which it sets to 1
    HasPtr(const std::string &s = std::string()):
        ps(new std::string(s)), i(0), use(new std::size_t(1))
    {}
    // copy constructor copies all three data members and increments the counter
    HasPtr(const HasPtr &p):
    ps(p.ps), i(p.i), use(p.use) { ++*use; }
    HasPtr& operator=(const HasPtr&);
    ~HasPtr();
private:
    std::string *ps;
    int i;
    std::size_t *use; // member to keep track of how many objects share *ps
};

HasPtr::~HasPtr()
{
    if (--*use == 0) { // if the reference count goes to 0
        delete ps; // delete the string
        delete use; // and the counter
    }
}

HasPtr& HasPtr::operator=(const HasPtr &rhs)
{
    ++*rhs.use; // increment the use count of the right-hand operand
    if (--*use == 0) { // then decrement this object's counter
        delete ps; // if no other users
        delete use; // free this object's allocated members
    }
    ps = rhs.ps; // copy data from rhs into this object
    i = rhs.i;
    use = rhs.use;
    return *this; // return this object
}
```

## 交换操作

```c++
class HasPtr {
    friend void swap(HasPtr&, HasPtr&);
    // other members as in § 13.2.1
};

// 每个swap调用应该都是未加限定的，应该是swap，而不是std::swap。
inline void swap(HasPtr &lhs, HasPtr &rhs)
{
    using std::swap;
    swap(lhs.ps, rhs.ps); // swap the pointers, not the string data
    swap(lhs.i, rhs.i); // swap the int members
}
```

#### 拷贝并交换技术

```c++
// note rhs is passed by value, which means the HasPtr copy constructor
// copies the string in the right-hand operand into rhs
HasPtr& HasPtr::operator=(HasPtr rhs)
{
    // swap the contents of the left-hand operand with the local variable rhs
    swap(*this, rhs); // rhs now points to the memory this object had used
    return *this; // rhs is destroyed, which deletes the pointer in rhs
}
```

## 拷贝控制示例

* `Message类`：表示消息，可以出现在多个`Folder`中，内容只有一个副本；如果一条`Message`的内容被改变，则从它所在的任何`Folder`来浏览此`Message`，都会看到改变的内容
* `Folder类`：表示消息目录

[message_folder_main.cpp](code/message_folder_main.cpp)


## 动态内存管理类

某些类需要在运行时分配可变大小的内存空间，通常可以使用标准库容器来保存数据。如果需要自己进行内存分配，一般来说必须定义自己的拷贝控制成员来管理所分配的内存。

### StrVec

实现标准库vector类的一个简化版本，只用于string

[StrVec](code/strvec.cpp)

## 对象移动

某些情况下，对象拷贝后就立即被销毁了。在这些情况下，移动而非拷贝对象会大幅提升性能

IO类或unique_ptr这样的类包含不能被共享的资源，这些类型的对象不能拷贝但可以移动

**容器中可以保存不可拷贝的类型，只要它们能被移动即可**

### 右值引用

> C++11：为了支持移动操作，引入右值引用

右值引用必须绑定到右值的引用，只能绑定到一个将要销毁的对象

```c++
int &r = i; // ok: r refers to i
int &&rr = i; // error: cannot bind an rvalue reference to an lvalue
int &r2 = i * 42; // error: i * 42 is an rvalue
const int &r3 = i * 42; // ok: we can bind a reference to const to an rvalue
int &&rr2 = i * 42; // ok: bind rr2 to the result of the multiplication
// 变量是左值
int &&rr1 = 42; // ok: literals are rvalues
int &&rr2 = rr1; // error: the expression rr1 is an lvalue!
```

可以将一个const的左值引用或者一个右值引用绑定到生成右值的表达式上

#### 标准库move函数

显式地将一个左值转换为对应的右值引用类型

> C++11：调用move来获得绑定到左值上的右值引用

```c++
int &&rr3 = std::move(rr1); // ok
```

**可以销毁一个移后源对象，也可以赋予新值，但不能使用一个移后源对象的值**

### 移动构造函数和移动赋值运算符

#### 移动构造函数

一旦资源完成移动，源对象必须不再指向被移动的资源，这些资源的所有权已经归属新创建的对象

```c++
class StrVec {
public:
    StrVec(StrVec&&) noexcept; // move constructor
    // other members as before
};

StrVec::StrVec(StrVec &&s) noexcept // move won't throw any exceptions
// member initializers take over the resources in s
    : elements(s.elements), first_free(s.first_free), cap(s.cap)
{
    // leave s in a state in which it is safe to run the destructor
    s.elements = s.first_free = s.cap = nullptr;
}
```

#### 移动赋值运算符

```c++
StrVec &StrVec::operator=(StrVec &&rhs) noexcept
{
    // direct test for self-assignment
    if (this != &rhs) {
        free(); // free existing elements
        elements = rhs.elements; // take over resources from rhs
        first_free = rhs.first_free;
        cap = rhs.cap;
        // leave rhs in a destructible state
        rhs.elements = rhs.first_free = rhs.cap = nullptr;
    }
    return *this;
}
```

#### 移后源对象必须可析构

移后源对象还是有效的，但对留下的值不应做任何假设

#### 合成的移动操作

如果一个类定义了自己的拷贝构造函数、拷贝赋值运算符或者析构函数，编译器不会为它合成移动构造函数和移动赋值运算符

只有当一个类没有定义任何自己版本的拷贝控制成员，且类的每个非static数据成员都可以移动时，编译器才会为它合成移动构造函数或移动赋值运算符

```c++
// the compiler will synthesize the move operations for X and hasX
struct X {
    int i; // built-in types can be moved
    std::string s; // string defines its own move operations
};
struct hasX {
    X mem; // X has synthesized move operations
};
X x, x2 = std::move(x); // uses the synthesized move constructor
hasX hx, hx2 = std::move(hx); // uses the synthesized move constructor
```

移动操作不会隐式定义为删除的函数。如果显式地要求编译器生成=default的移动操作，且编译器不能移动所有成员，则编译器会将移动操作定义为删除的函数

```c++
// assume Y is a class that defines its own copy constructor but not a move constructor
struct hasY {
    hasY() = default;
    hasY(hasY&&) = default;
    Y mem; // hasY will have a deleted move constructor
};
hasY hy, hy2 = std::move(hy); // error: move constructor is deleted
```

#### 移动右值，拷贝左值

```c++
StrVec v1, v2;
v1 = v2; // v2 is an lvalue; copy assignment
StrVec getVec(istream &); // getVec returns an rvalue
v2 = getVec(cin); // getVec(cin) is an rvalue; move assignment
```

#### 没有移动构造函数，右值也被拷贝

用拷贝构造函数代替移动构造函数几乎肯定是安全的

```c++
class Foo {
public:
    Foo() = default;
    Foo(const Foo&); // copy constructor
    // other members, but Foo does not define a move constructor
};
Foo x;
Foo y(x); // copy constructor; x is an lvalue
Foo z(std::move(x)); // copy constructor, because there is no move constructor
```

#### 拷贝并交换赋值运算符和移动操作

```c++
class HasPtr {
public:
    // added move constructor
    HasPtr(HasPtr &&p) noexcept : ps(p.ps), i(p.i) {
        p.ps =0;
    }
    // assignment operator is both the move- and copy-assignment operator
    HasPtr& operator=(HasPtr rhs)
    {
        swap(*this, rhs); 
        return *this;
    }
    // other members as in § 13.2.1
};

hp = hp2; // hp2 is an lvalue; copy constructor used to copy hp2
hp = std::move(hp2); // move constructor moves hp2
```

[Message类的移动操作](code/message_folder_main.cpp)

#### 移动迭代器

* 普通迭代器解引用返回指向元素的左值
* 移动迭代器解引用生成一个右值引用

> C++11: `make_move_iterator`将一个普通迭代器转换为一个移动迭代器

```c++
void StrVec::reallocate()
{
    // allocate space for twice as many elements as the current size
    auto newcapacity = size() ? 2 * size() : 1;
    auto first = alloc.allocate(newcapacity);
    // move the elements
    auto last = uninitialized_copy(make_move_iterator(begin()),
                                   make_move_iterator(end()),
                                   first);
    free(); // free the old space
    elements = first; // update the pointers
    first_free = last;
    cap = elements + newcapacity;
}
```

### 右值引用和成员函数

成员函数同时提供拷贝和移动版本，拷贝版本接受一个指向const的左值引用，移动版本接受一个指向非const的右值引用

```c++
void push_back(const X&); // copy: binds to any kind of X
void push_back(X&&); // move: binds only to modifiable rvalues of type X

class StrVec {
public:
    void push_back(const std::string&); // copy the element
    void push_back(std::string&&); // move the element
    // other members as before
};

// unchanged from the original version in § 13.5
void StrVec::push_back(const string& s)
{
    chk_n_alloc(); // ensure that there is room for another element
    // construct a copy of s in the element to which first_free points
    alloc.construct(first_free++, s);
}

void StrVec::push_back(string &&s)
{
    chk_n_alloc(); // reallocates the StrVec if necessary
    alloc.construct(first_free++, std::move(s));
}

StrVec vec; // empty StrVec
string s = "some string or another";
vec.push_back(s); // calls push_back(const string&)
vec.push_back("done"); // calls push_back(string&&)
```

#### 右值和左值引用成员函数

指出this的左值右值属性的方式：在参数列表后放置一个引用限定符，&或&&，分别指出this可以指向一个左值或右值

```c++
class Foo {
public:
    Foo &operator=(const Foo&) &; // may assign only to modifiable lvalues
    // other members of Foo
};

Foo &Foo::operator=(const Foo &rhs) &
{
    // do whatever is needed to assign rhs to this object
    return *this;
}

Foo &retFoo(); // returns a reference; a call to retFoo is an lvalue
Foo retVal(); // returns by value; a call to retVal is an rvalue
Foo i, j; // i and j are lvalues
i = j; // ok: i is an lvalue
retFoo() = j; // ok: retFoo() returns an lvalue
retVal() = j; // error: retVal() returns an rvalue
i = retVal(); // ok: we can pass an rvalue as the right-hand operand to assignment

class Foo {
public:
    Foo someMem() & const; // error: const qualifier must come first
    Foo anotherMem() const &; // ok: const qualifier comes first
};
```

#### 重载和引用函数

综合引用限定符和const来区分一个成员函数的重载版本

```c++
class Foo {
public:
    Foo sorted() &&; // may run on modifiable rvalues
    Foo sorted() const &; // may run on any kind of Foo
    // other members of Foo
private:
    vector<int> data;
};
// this object is an rvalue, so we can sort in place
Foo Foo::sorted() &&
{
    sort(data.begin(), data.end());
    return *this;
}
// this object is either const or it is an lvalue; either way we can't sort in place
Foo Foo::sorted() const & {
    Foo ret(*this); // make a copy
    sort(ret.data.begin(), ret.data.end()); // sort the copy
    return ret; // return the copy
}

retVal().sorted(); // retVal() is an rvalue, calls Foo::sorted() &&
retFoo().sorted(); // retFoo() is an lvalue, calls Foo::sorted() const &

// 定义具有相同名字和相同参数列表的成员函数，必须对所有成员函数都加上引用限定符或者所有都不加
class Foo {
public:
    Foo sorted() &&;
    Foo sorted() const; // error: must have reference qualifier
    // Comp is type alias for the function type (see § 6.7)
    // that can be used to compare int values
    using Comp = bool(const int&, const int&);
    Foo sorted(Comp*); // ok: different parameter list
    Foo sorted(Comp*) const; // ok: neither version is reference qualified
};
```