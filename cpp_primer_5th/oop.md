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

## 抽象基类

## 访问控制与继承

## 继承中的类作用域

## 容器与继承