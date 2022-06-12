# 面向对象程序设计

## OOP概述

核心思想：数据抽象、继承、动态绑定

#### 继承

派生类列表(class derivation list)

```c++
// 按原价销售的书籍
class Quote {
public:
    std::string isbn() const;
    virtual double net_price(std::size_t n) const;
};

// 可以打折销售的书籍
class Bulk_quote : public Quote { // Bulk_quote inherits from Quote
public:
    double net_price(std::size_t) const override;
};
```

#### 动态绑定

```c++
// calculate and print the price for the given number of copies, applying any discounts
double print_total(ostream &os, const Quote &item, size_t n)
{
    // depending on the type of the object bound to the item parameter
    // calls either Quote::net_price or Bulk_quote::net_price
    double ret = item.net_price(n);
    os << "ISBN: " << item.isbn() // calls Quote::isbn
    << " # sold: " << n << " total due: " << ret << endl;
    return ret;
}

// basic has type Quote; bulk has type Bulk_quote
print_total(cout, basic, 20); // calls Quote version of net_price
print_total(cout, bulk, 20); // calls Bulk_quote version of net_price
```

## 定义基类和派生类

### 定义基类

```c++
class Quote {
public:
    Quote() = default; // = default see § 7.1.4
    Quote(const std::string &book, double sales_price):
        bookNo(book), price(sales_price) { }
    std::string isbn() const { return bookNo; }
    // returns the total sales price for the specified number of items
    // derived classes will override and apply different discount algorithms
    virtual double net_price(std::size_t n) const
    {
        return n * price;
    }
    virtual ~Quote() = default; // dynamic binding for the destructor
private:
    std::string bookNo; // ISBN number of this item
protected:
    double price = 0.0; // normal, undiscounted price
};
```

派生类可以继承定义在基类中的成员，但是派生类的成员函数不一定有权访问从基类继承而来的成员。派生类能访问公有成员，不能访问私有成员

protected成员：基类希望它的派生类有权访问该成员，同时禁止其他用户访问

### 定义派生类

```c++
class Bulk_quote : public Quote { // Bulk_quote inherits from Quote
public:
    Bulk_quote() = default;
    Bulk_quote(const std::string&, double, std::size_t, double);
    // overrides the base version in order to implement the bulk purchase discount policy
    double net_price(std::size_t) const override;
private:
    std::size_t min_qty = 0; // minimum purchase for the discount to apply
    double discount = 0.0; // fractional discount to apply
};
```

**基类前的访问说明符：控制派生类从基类继承而来的成员是否对派生类的用户可见**

如果一个派生是public，则基类的公有成员也是派生类接口的组成部分，可以将公有派生类型的对象绑定到基类的引用或指针上

#### 派生类向基类的类型转换

```c++
Quote item; // object of base type
Bulk_quote bulk; // object of derived type
Quote *p = &item; // p points to a Quote object
p = &bulk; // p points to the Quote part of bulk
Quote &r = bulk; // r bound to the Quote part of bulk
```

#### 派生类构造函数

派生类必须使用基类的构造函数初始化它的基类部分

```c++
Bulk_quote(const std::string& book, double p, std::size_t qty, double disc) :
    Quote(book, p), min_qty(qty), discount(disc) { }
    // as before
```

#### 派生类使用基类的成员

可以访问基类的公有成员和受保护成员

```c++
// if the specified number of items are purchased, use the discounted price
double Bulk_quote::net_price(size_t cnt) const
{
    if (cnt >= min_qty)
        return cnt * (1 - discount) * price;
    else
        return cnt * price;
}
```

#### 继承与静态成员

如果基类定义了一个静态成员，则在整个继承体系中只存在该成员的唯一定义

```c++
class Base {
public:
    static void statmem();
};
class Derived : public Base {
    void f(const Derived&);
};

void Derived::f(const Derived &derived_obj)
{
    Base::statmem(); // ok: Base defines statmem
    Derived::statmem(); // ok: Derived inherits statmem
    // ok: derived objects can be used to access static from base
    derived_obj.statmem(); // accessed through a Derived object
    statmem(); // accessed through this object
}
```

#### 被用作基类的类

必须已经定义而非仅仅声明

```c++
class Quote; // declared but not defined
// error: Quote must be defined
class Bulk_quote : public Quote { ... };
```

#### 防止继承

> C++11：final

```c++
class NoDerived final { /* */ }; // NoDerived can't be a base class
class Base { /* */ };
// Last is final; we cannot inherit from Last
class Last final : Base { /* */ }; // Last can't be a base class
class Bad : NoDerived { /* */ }; // error: NoDerived is final
class Bad2 : Last { /* */ }; // error: Last is final
```

### 类型转换与继承

可以将基类的指针或引用绑定到派生类对象上

#### 不存在从基类向派生类的隐式类型转换

```c++
Quote base;
Bulk_quote* bulkP = &base; // error: can't convert base to derived
Bulk_quote& bulkRef = base; // error: can't convert base to derived

Bulk_quote bulk;
Quote *itemP = &bulk; // ok: dynamic type is Bulk_quote
Bulk_quote *bulkP = itemP; // error: can't convert base to derived
```

* 运行时执行类型转换：`dynamic_cast`
* 已知基类向派生类的转换是安全的：`static_cast`

#### 对象之间不存在类型转换

用一个派生类对象为一个基类对象初始化或赋值时，只有该派生类对象中的基类部分会被拷贝、移动或赋值，它的派生类部分将被忽略

```c++
Bulk_quote bulk; // object of derived type
Quote item(bulk); // uses the Quote::Quote(const Quote&) constructor
item = bulk; // calls Quote::operator=(const Quote&)
```

## 虚函数

运行时才能知道到底调用了哪个版本的虚函数，所以所有虚函数都必须有定义

#### 对虚函数的调用可能在运行时才被解析

动态绑定只有当通过引用或指针调用虚函数时才会发生

引用或指针的静态类型与动态类型不同是C++支持多态性的根本所在

#### 派生类中的虚函数

一旦某个函数被声明为虚函数，则在所有派生类中它都是虚函数

#### final和override

> C++11：override标记派生类中的虚函数

```c++
struct B {
    virtual void f1(int) const;
    virtual void f2();
    void f3();
};

struct D1 : B {
    void f1(int) const override; // ok: f1 matches f1 in the base
    void f2(int) override; // error: B has no f2(int) function
    void f3() override; // error: f3 not virtual
    void f4() override; // error: B doesn't have a function named f4
};
```

final：如果已经定义为final，则之后任何尝试覆盖该函数的操作都将引发错误

```c++
struct D2 : B {
    // inherits f2() and f3() from B and overrides f1(int)
    void f1(int) const final; // subsequent classes can't override f1(int)
};
struct D3 : D2 {
    void f2(); // ok: overrides f2 inherited from the indirect base, B
    void f1(int) const; // error: D2 declared f2 as final
};
```

#### 虚函数与默认实参

默认实参值由静态类型决定。如果使用基类的指针或引用调用函数，则使用基类中定义的默认实参

#### 回避虚函数机制

强制执行虚函数的某个特定版本

```c++
// calls the version from the base class regardless of the dynamic type of baseP
double undiscounted = baseP->Quote::net_price(42);
```

## 抽象基类

#### 纯虚函数

纯虚函数无须定义

```c++
// class to hold the discount rate and quantity
// derived classes will implement pricing strategies using these data
class Disc_quote : public Quote {
public:
    Disc_quote() = default;
    Disc_quote(const std::string& book, double price, std::size_t qty, double disc):
        Quote(book, price), quantity(qty), discount(disc) { }
    double net_price(std::size_t) const = 0;
protected:
    std::size_t quantity = 0; // purchase size for the discount to apply
    double discount = 0.0; // fractional discount to apply
};
```

#### 含有纯虚函数的类或者未经覆盖直接继承纯虚函数的类是抽象基类

抽象基类负责定义接口，不能直接创建一个抽象基类的对象

```c++
// Disc_quote declares pure virtual functions, which Bulk_quote will override
Disc_quote discounted; // error: can't define a Disc_quote object
Bulk_quote bulk; // ok: Bulk_quote has no pure virtual functions
```

#### 派生类的构造函数只初始化它的直接基类

```c++
// the discount kicks in when a specified number of copies of the same book are sold
// the discount is expressed as a fraction to use to reduce the normal price
class Bulk_quote : public Disc_quote {
public:
    Bulk_quote() = default;
    Bulk_quote(const std::string& book, double price, std::size_t qty, double disc):
        Disc_quote(book, price, qty, disc) { }
    // overrides the base version to implement the bulk purchase discount policy
    double net_price(std::size_t) const override;
};
```

## 访问控制与继承

每个类分别控制自己的成员初始化过程，还分别控制着其成员对于派生类来说是否可访问

### 受保护的成员

一个类使用protected关键字声明那些它希望与派生类分享但是不想被其他公共访问使用的成员
* 受保护的成员对于类的用户来说不可访问
* 受保护的成员对于派生类的成员和友元来说是可访问的
* 派生类的成员或友元只能通过派生类对象来访问基类的受保护成员

```c++
class Base {
protected:
    int prot_mem; // protected member
};

class Sneaky : public Base {
    friend void clobber(Sneaky&); // can access Sneaky::prot_mem
    friend void clobber(Base&); // can't access Base::prot_mem
    int j; // j is private by default
};

// ok: clobber can access the private and protected members in Sneaky objects
void clobber(Sneaky &s) { s.j = s.prot_mem = 0; }
// error: clobber can't access the protected members in Base
void clobber(Base &b) { b.prot_mem = 0; }
```

### 公有、私有和受保护继承

某个类对其继承而来的成员的访问权限受到两个因素的影响：
* 基类中该成员的访问说明符
* 派生类的派生列表中的访问说明符

```c++
class Base {
public:
    void pub_mem(); // public member
protected:
    int prot_mem; // protected member
private:
    char priv_mem; // private member
};

struct Pub_Derv : public Base {
    // ok: derived classes can access protected members
    int f() { return prot_mem; }
    // error: private members are inaccessible to derived classes
    char g() { return priv_mem; }
};

struct Priv_Derv : private Base {
    // private derivation doesn't affect access in the derived class
    int f1() const { return prot_mem; }
};

Pub_Derv d1; // members inherited from Base are public
Priv_Derv d2; // members inherited from Base are private
d1.pub_mem(); // ok: pub_mem is public in the derived class
d2.pub_mem(); // error: pub_mem is private in the derived class
```

派生访问说明符对于**派生类的成员及友元**能否访问其**直接基类**的成员没什么影响。

对基类成员的访问权限只与**基类中的访问说明**符有关。

派生访问说明符的目的是控制派生类用户（包括派生类的派生类在内）对于基类成员的访问权限：
* 继承是共有的，则成员将遵循其原有的访问说明符
* 继承是私有的，则成员是私有的
* 继承是受保护的，则所有公有成员在新定义的类中都是受保护的

```c++
struct Derived_from_Public : public Pub_Derv {
    // ok: Base::prot_mem remains protected in Pub_Derv
    int use_base() { return prot_mem; }
};
struct Derived_from_Private : public Priv_Derv {
    // error: Base::prot_mem is private in Priv_Derv
    int use_base() { return prot_mem; }
};
```

## 继承中的类作用域

## 构造函数与拷贝控制

## 容器与继承

