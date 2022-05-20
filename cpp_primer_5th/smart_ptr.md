# 动态内存与智能指针

* 静态内存：局部static变量、类static数据成员、定义在任何函数之外的变量
* 栈内存：函数内非static对象
* 堆内存：动态分配的对象

## 动态内存与智能指针

> C++11：智能指针

* shared_ptr：允许多个指针指向同一个对象
* unique_ptr：独占所指向的对象
* wear_ptr：伴随类，弱引用，指向shared_ptr所管理的对象

### shared_ptr类

```c++
// 定义
shared_ptr<string> p1; // shared_ptr that can point at a string
shared_ptr<list<int>> p2; // shared_ptr that can point at a list of ints

// 使用
// if p1 is not null, check whether it's the empty string
if (p1 && p1->empty())
    *p1 = "hi"; // if so, dereference p1 to assign a new value to that string
```

默认初始化的智能指针中保存着一个空指针

**shared_ptr和unique_ptr都支持的操作**

| 操作 <div style="width:200px"> | 解释 |
| ---  | ---  |
| `shared_ptr<T> sp` <br> `unique_ptr<T> up` | 空智能指针，可以指向类型为T的对象 |
| `p` | 将`p`用作一个条件判断，若`p`指向一个对象，则为true |
| `*p` | 解引用`p`，获得它指向的对象 |
| `p->mem` | 等价于(*p).mem |
| `p.get()` | 返回p中保存的指针。要小心使用，若智能指针释放了其对象，返回的指针所指向的对象也消失了 |
| `swap(p, q)` <br> `p.swap(q)` | 交换p和q中的指针 |

**shared_ptr独有的操作**

| 操作 <div style="width:220px"> | 解释 |
| ---  | ---  |
| `make_shared<T>(args)` | 返回一个shared_ptr，指向一个动态分配的类型为T的对象，使用args初始化此对象 |
| `shared_ptr<T> p(q)` | p是shared_ptr q的拷贝：此操作会递增q中的计数器，q中的指针必须能转换为T* |
| `p = q` | p和q都是shared_ptr，所保存的指针必须能互相转换，此操作会递减p的引用计数，递增q的引用计数；若p的引用计数变为0，则将其管理的原内存释放 |
| `p.unique()` | 若p.use_count()为1，返回true；否则返回false |
| `p.use_count()` | 返回与p共享对象的智能指针数量；可能很慢，主要用于调试 |

#### make_shared函数

```c++
// shared_ptr that points to an int with value 42
shared_ptr<int> p3 = make_shared<int>(42);
// p4 points to a string with value 9999999999
shared_ptr<string> p4 = make_shared<string>(10, '9');
// p5 points to an int that is value initialized (§ 3.3.1 (p. 98)) to 0
shared_ptr<int> p5 = make_shared<int>();
// p6 points to a dynamically allocated, empty vector<string>
auto p6 = make_shared<vector<string>>();
```

#### shared_ptr的拷贝和赋值

每个shared_ptr都有一个关联的计数器，通常称为**引用计数**

```c++
auto p = make_shared<int>(42); // object to which p points has one user
auto q(p); // p and q point to the same object
// object to which p and q point has two users

auto r = make_shared<int>(42); // int to which r points has one user
r = q; // assign to r, making it point to a different address
// increase the use count for the object to which q points
// reduce the use count of the object to which r had pointed
// the object r had pointed to has no users; that object is automatically freed
```

#### shared_ptr自动销毁所管理的对象

shared_ptr的析构函数会递减它所指向的对象的引用计数，如果引用计数变为0，shared_ptr的析构函数就会销毁对象，并释放它占用的内存

#### shared_ptr自动释放相关联的内存

```c++
// factory returns a shared_ptr pointing to a dynamically allocated object
shared_ptr<Foo> factory(T arg)
{
    // process arg as appropriate
    // shared_ptr will take care of deleting this memory
    return make_shared<Foo>(arg);
}

void use_factory(T arg)
{
    shared_ptr<Foo> p = factory(arg);
    // use p
} // p goes out of scope; the memory to which p points is automatically freed

shared_ptr<Foo> use_factory(T arg)
{
    shared_ptr<Foo> p = factory(arg);
    // use p
    return p; // reference count is incremented when we return p
} // p goes out of scope; the memory to which p points is not freed
```

#### 使用了动态生存期的资源的类

程序使用动态内存的原因：

* 程序不知道自己需要使用多少对象，例如容器
* 程序不知道所需对象的准确类型
* 程序需要载多个对象间共享数据

分配的资源与对应对象生存期一致。例如vector拥有自己的元素

```c++
vector<string> v1; // empty vector
{ // new scope
    vector<string> v2 = {"a", "an", "the"};
    v1 = v2; // copies the elements from v2 into v1
} // v2 is destroyed, which destroys the elements in v2
// v1 has three elements, which are copies of the ones originally in v2
```

某些类分配的资源具有与原对象相独立的生存期

```c++
// 定义一个名为Blob的类，保存一组元素。Blob对象的不同拷贝之间共享相同的元素

Blob<string> b1; // empty Blob
{ // new scope
    Blob<string> b2 = {"a", "an", "the"};
    b1 = b2; // b1 and b2 share the same elements
} // b2 is destroyed, but the elements in b2 must not be destroyed
// b1 points to the elements originally created in b2
```

#### 定义Blob类

先定义一个管理string的类StrBlob

```c++
class StrBlob {
public:
    typedef std::vector<std::string>::size_type size_type;
    StrBlob();
    StrBlob(std::initializer_list<std::string> il);
    size_type size() const { return data->size(); }
    bool empty() const { return data->empty(); }
    // add and remove elements
    void push_back(const std::string &t) { data->push_back(t); }
    void pop_back();
    // element access
    std::string& front();
    std::string& back();
private:
    std::shared_ptr<std::vector<std::string>> data;
    // throws msg if data[i] isn't valid
    void check(size_type i, const std::string &msg) const;
};

// 构造函数
StrBlob::StrBlob(): data(make_shared<vector<string>>()) { }
StrBlob::StrBlob(initializer_list<string> il): data(make_shared<vector<string>>(il)) { }

void StrBlob::check(size_type i, const string &msg) const
{
    if (i >= data->size())
        throw out_of_range(msg);
}

// 元素访问成员函数
string& StrBlob::front()
{
    // if the vector is empty, check will throw
    check(0, "front on empty StrBlob");
    return data->front();
}
string& StrBlob::back()
{
    check(0, "back on empty StrBlob");
    return data->back();
}
void StrBlob::pop_back()
{
    check(0, "pop_back on empty StrBlob");
    data->pop_back();
}
```

### shared_ptr和new结合使用

可以用new返回的指针来初始化智能指针，接受指针参数的智能指针构造函数是explict的，不能讲一个内置指针隐式转换为一个智能指针，必须使用直接初始化形式。一个返回shared_ptr的函数不能在其返回语句中隐式转换为一个普通指针

```c++
shared_ptr<double> p1; // shared_ptr that can point at a double
shared_ptr<int> p2(new int(42)); // p2 points to an int with value 42

shared_ptr<int> p1 = new int(1024); // error: must use direct initialization
shared_ptr<int> p2(new int(1024)); // ok: uses direct initialization

shared_ptr<int> clone(int p) {
    return new int(p); // error: implicit conversion to shared_ptr<int>
}

shared_ptr<int> clone(int p) {
    // ok: explicitly create a shared_ptr<int> from int*
    return shared_ptr<int>(new int(p));
}
```

**定义和改变shared_ptr的其他方法**

| 操作 <div style="width:200px"> | 解释 |
| --- | --- |
| `shared_ptr<T> p(q)` | p管理内置指针q所指向的对象；q必须指向new分配的内存，且能够转换为T*类型 |
| `shared_ptr<T> p(u)` | p从unique_ptr u那里接管了对象的所有权；将u置为空 |
| `shared_ptr<T> p(q, d)` | p接管了内置指针q所指向的对象的所有权。q必须能转换为T*类型。p将使用可调用对象d来代替delete |
| `shared_ptr<T> p(p2, d)` | p是shared_ptr p2的拷贝，唯一的区别是p将用可调用对象d来代替delete |
| `p.reset()` <br> `p.reset(q)` <br> `p.reset(q, d)` | 若p是唯一指向其对象的shared_ptr，reset会释放此对象。若传递了可选的参数内置指针q，会令p指向q，否则会将p置为空。若还传递了参数d，将会调用d而不是delete来释放q |

#### 不要混合使用普通指针和智能指针

```c++
// ptr is created and initialized when process is called
void process(shared_ptr<int> ptr)
{
    // use ptr
} // ptr goes out of scope and is destroyed

shared_ptr<int> p(new int(42)); // reference count is 1
process(p); // copying p increments its count; in process the reference count is 2
int i = *p; // ok: reference count is 1

int *x(new int(1024)); // dangerous: x is a plain pointer, not a smart pointer
process(x); // error: cannot convert int* to shared_ptr<int>
process(shared_ptr<int>(x)); // legal, but the memory will be deleted!
int j = *x; // undefined: x is a dangling pointer!
```

**不应该再使用内置指针来访问shared_ptr所指向的内存**

#### 不要使用get初始化另一个之智能指针或为智能指针赋值

get函数返回一个内置指针，指向智能指针管理的对象，用于需要向不使用智能指针的代码传递一个内置指针。

**使用get返回的指针的代码不能delete此指针，将另一个智能指针也绑定到get返回的指针上是错误的**

```c++
shared_ptr<int> p(new int(42)); // reference count is 1
int *q = p.get(); // ok: but don't use q in any way that might delete its pointer

{ // new block
    // undefined: two independent shared_ptrs point to the same memory
    shared_ptr<int>(q);
} // block ends, q is destroyed, and the memory to which q points is freed

int foo = *p; // undefined; the memory to which p points was freed
```

#### 其他shared_ptr操作

```c++
if (!p.unique())
    p.reset(new string(*p)); // we aren't alone; allocate a new copy
*p += newVal; // now that we know we're the only pointer, okay to change this object
```

### 智能指针和异常

智能指针是一个简单的确保资源被释放的方法

```c++
void f()
{
    shared_ptr<int> sp(new int(42)); // allocate a new object
    // code that throws an exception that is not caught inside f
} // shared_ptr freed automatically when the function ends
```

发生异常时，直接管理的内存不会自动释放

```c++
void f()
{
    int *ip = new int(42); // dynamically allocate a new object
    // code that throws an exception that is not caught inside f
    delete ip; // free the memory before exiting
}
```

#### 智能指针和哑类

为C和C++两种语言设计的类，可能没有定义析构函数，通常要求用户显示地释放所使用的资源

```c++
struct destination; // represents what we are connecting to
struct connection; // information needed to use the connection
connection connect(destination*); // open the connection
void disconnect(connection); // close the given connection
void f(destination &d /* other parameters */)
{
    // get a connection; must remember to close it when done
    connection c = connect(&d);
    // use the connection
    // if we forget to call disconnect before exiting f, there will be no way to close c
}

// connection没有析构函数
```

#### 使用我们自己的释放操作

```c++
// 定义删除器
void end_connection(connection *p) { disconnect(*p); }

// 创建shared_ptr时，可以传递一个指向删除器函数的参数
void f(destination &d /* other parameters */)
{
    connection c = connect(&d);
    shared_ptr<connection> p(&c, end_connection);
    // use the connection
    // when f exits, even if by an exception, the connection will be properly closed
}
```

### unique_ptr

一个unique_ptr拥有它所指向的对象。定义一个unique_ptr时，需要将其绑定到一个new返回的指针上，必须直接初始化

```c++
unique_ptr<double> p1; // unique_ptr that can point at a double
unique_ptr<int> p2(new int(42)); // p2 points to int with value 42

unique_ptr<string> p1(new string("Stegosaurus"));
unique_ptr<string> p2(p1); // error: no copy for unique_ptr
unique_ptr<string> p3;
p3 = p2; // error: no assign for unique_ptr
```

**unique_ptr支持的操作**

| 操作 <div style="width:200px"> | 解释 |
| --- | --- |
| `unique_ptr<T> u1` | 空unique_ptr，可以指向类型为T的对象。u1会使用delete释放它的指针 |
| `unique_ptr<T, D> u2` | u2会使用一个类型为D的可调用对象释放它的指针 |
| `unique_ptr<T, D> u(d)` | 空unique_ptr，可以指向类型为T的对象。用类型为D的对象d来代替delete |
| `u = nullptr` | 释放u指向的对象，将u置为空 |
| `u.release()` | u释放对指针的控制权，返回指针，并将u置为空 |
| `u.reset()` <br> `u.reset(q)` <br> `u.reset(nullptr)` | 释放u指向的对象。如果提供了内置指针q，令u指向这个对象；否则将u置为空 |

```c++
// transfers ownership from p1 (which points to the string Stegosaurus) to p2
unique_ptr<string> p2(p1.release()); // release makes p1 null
unique_ptr<string> p3(new string("Trex"));
// transfers ownership from p3 to p2
p2.reset(p3.release()); // reset deletes the memory to which p2 had pointed

p2.release(); // WRONG: p2 won't free the memory and we've lost the pointer
auto p = p2.release(); // ok, but we must remember to delete(p)
```

#### 传递unique_ptr参数和返回unique_ptr

不能拷贝unique_ptr有一个例外：可以拷贝或赋值一个将要被销毁的unique_ptr

```c++
unique_ptr<int> clone(int p) {
    // ok: explicitly create a unique_ptr<int> from int*
    return unique_ptr<int>(new int(p));
}

unique_ptr<int> clone(int p) {
    unique_ptr<int> ret(new int(p));
    // . . .
    return ret;
}
```

#### 向unique_ptr传递删除器

```c++
// p points to an object of type objT and uses an object of type delT to free that object
// it will call an object named fcn of type delT
unique_ptr<objT, delT> p (new objT, fcn);

void f(destination &d /* other needed parameters */)
{
    connection c = connect(&d); // open the connection
    // when p is destroyed, the connection will be closed
    unique_ptr<connection, decltype(end_connection)*>
    p(&c, end_connection);
    // use the connection
    // when f exits, even if by an exception, the connection will be properly closed
}
```

### weak_ptr

不控制所指向对象生存期的智能指针，指向由一个shared_ptr管理的对象，不会改变shared_ptr的引用计数

**weak_ptr支持的操作**

| 操作 <div style="width:200px"> | 解释 |
| --- | --- |
| `weak_ptr<T> w` | 空weak_ptr可以指向类型为T的对象 |
| `weak_ptr<T> w(sp)` | 与shared_ptr sp指向相同对象的weak_ptr。T必须能够转换为sp指向的类型 |
| `w = p` | p可以是一个shared_ptr或一个weak_ptr。赋值后w与p共享对象 |
| `w.reset()` | 将w置为空 |
| `w.use_count()` | 与w共享对象的shared_ptr的数量 |
| `w.expired()` | 若w.use_count()为0，返回true，否则返回false |
| `w.lock()` | 如果expired为true，返回一个空shared_ptr；否则返回一个指向w的对象的shared_ptr |

```c++
auto p = make_shared<int>(42);
weak_ptr<int> wp(p); // wp weakly shares with p; use count in p is unchanged

if (shared_ptr<int> np = wp.lock()) { // true if np is not null
    // inside the if, np shares its object with p
}
```

#### 使用weak_ptr为StrBlob类定义一个伴随指针类StrBlobPtr

```c++
// StrBlobPtr throws an exception on attempts to access a nonexistent element
class StrBlobPtr {
public:
    StrBlobPtr(): curr(0) { }
    StrBlobPtr(StrBlob &a, size_t sz = 0) : wptr(a.data), curr(sz) { }
    std::string& deref() const;
    StrBlobPtr& incr(); // prefix version
private:
    // check returns a shared_ptr to the vector if the check succeeds
    std::shared_ptr<std::vector<std::string>> check(std::size_t, const std::string&) const;
    // store a weak_ptr, which means the underlying vector might be destroyed
    std::weak_ptr<std::vector<std::string>> wptr;
    std::size_t curr; // current position within the array
};

std::shared_ptr<std::vector<std::string>>
StrBlobPtr::check(std::size_t i, const std::string &msg) const
{
    auto ret = wptr.lock(); // is the vector still around?
    if (!ret)
        throw std::runtime_error("unbound StrBlobPtr");
    if (i >= ret->size())
        throw std::out_of_range(msg);
    return ret; // otherwise, return a shared_ptr to the vector
}

// defined functions named deref and incr to dereference and increment the StrBlobPtr, respectively.
std::string& StrBlobPtr::deref() const
{
    auto p = check(curr, "dereference past end");
    return (*p)[curr]; // (*p) is the vector to which this object points
}

// prefix: return a reference to the incremented object
StrBlobPtr& StrBlobPtr::incr()
{
    // if curr already points past the end of the container, can't increment it
    check(curr, "increment past end of StrBlobPtr");
    ++curr; // advance the current state
    return *this;
}

// forward declaration needed for friend declaration in StrBlob
class StrBlobPtr;
class StrBlob {
    friend class StrBlobPtr;
    // other members as in § 12.1.1
    // return StrBlobPtr to the first and one past the last elements
    StrBlobPtr begin() { return StrBlobPtr(*this); }
    StrBlobPtr end()
    { auto ret = StrBlobPtr(*this, data->size());
    return ret; }
};
```