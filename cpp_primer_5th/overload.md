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