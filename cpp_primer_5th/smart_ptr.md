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

| 操作 <div style="width:200px"> | 解释 |
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