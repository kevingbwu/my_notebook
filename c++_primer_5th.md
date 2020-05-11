# C++ Primer 5th Notes

## 容器

### 分类

* 顺序容器：array, vector, deque, forward_list, list, string(与vector类似，专门用于保存字符)
* 关联容器：set, multiset, map, multimap
* 无序关联容器：unordered_set, unordered_multiset, unordered_map, unordered_multimap
* 容器适配器：stack, queue, priority_queue

### 容器和容器适配器的区别

本质上，适配器是使某一类事物的行为类似于另一类事物的行为的一种机制。容器适配器让一种已存在的容器类型采用另一种不同的抽象类型的工作方式实现。

### 容器初始化

```c++
C c;                // 默认构造函数，为空

C c1(c2);
C c1 = c2;          // c1初始化为c2的拷贝

C c{a, b, c};
C c = {a, b, c};    // c初始化为初始化列表中元素的拷贝

C c(b, e);          // c初始化为迭代器b和e指定范围中的元素的拷贝

// 只有顺序容器（不包括array）的构造函数才能接受大小参数
C seq(n);           // seq包含n个元素，这些元素进行了值初始化，string不适用
C seq(n, t);        // seq包含n个初始化为值t的元素
```

### 容器赋值

```c++
c1 = c2;            // c1中的元素替换为c2中元素的拷贝，c1和c2必须具有相同的类型

c = {a,b,c,...};    // c中的元素替换为初始化列表中元素的拷贝

swap(c1, c2);       // 交换c1和c2中的元素，很快，常数时间
c1.swap(c2);

assign操作不适用于关联容器和array
seq.assign(b, e);   // seq中的元素替换为迭代器b和e所表示的范围中的元素
seq.assign(il);     // seq中的元素替换为初始化列表il中的元素
seq.assign(n, t);   // seq中的元素替换为n个值为t的元素
```

### 容器大小

* size: 不支持forward_list
* empty
* max_size

### 关系运算符

* 每个容器类型都支持：==, !=
* 除了无序关联容器外，都支持：>, >=, <, <=
* 容器使用元素的关系运算符实现比较

## 顺序容器

### 向顺序容器添加元素

```c++
// 这些操作会改变容器的大小；array不支持这些操作

// 在c的尾部创建一个值为t或有args创建的元素，返回void
c.push_back(t);
c.emplace_back(args);

// 在c的头部创建一个值为t或由args创建的元素，返回void
// vector和string不支持push_front和emplace_front
c.push_front(t);
c.emplace_front(args);

// 在迭代器p指向的元素之前创建一个值为t或由args创建的元素，返回指向新添加的元素的迭代器
c.insert(p, t);
c.emplace(p, args);

// p之前插入n个值为t的元素
// 将迭代器b和e指定范围内的元素添加到p之前
// initial list 插入到p指向的元素之前
c.insert(p, n, t);
c.insert(p, b, e);
c.insert(p, il);
```

**向vector、string、deque插入元素会使所有指向容器的迭代器、引用和指针失效。**

**容器元素是拷贝：用一个对象初始化容器或者将一个对象插入到容器，实际上放入到容器中的是对象值的一个拷贝，不是对象本身。**

**emplace函数在容器中直接构造元素，传递给emplace函数的参数必须与元素类型的构造函数相匹配**

### 访问元素

```c++
c.back();   // 返回c中尾元素的引用
c.front();  // 返回c中头元素的引用

// at和下标操作只适用于string、vector、deque、array
c[n];       // 返回c中下标为n的元素的引用
c.at(n);    // 返回c中下标为n的元素的引用
```

### 删除元素

```c++
c.pop_back();   // 删除c中的尾元素
c.pop_front();  // 删除c中的头元素，vector和string不支持
c.erase(p);     // 删除迭代器p所指向的元素，返回被删元素之后的迭代器
c.erase(b, e);  // 删除迭代器b和e所指定范围内的元素
c.clear();      // 删除c中所有元素
```

### 改变容器大小

```c++
c.resize(n);    // 调整c的大小为n个元素
c.resize(n, t); // 调整c的大小为n个元素，新添加的元素初始化为值t
```

### 容器操作可能使迭代器失效

### vector对象如何增长

```c++
// 容器大小管理操作
c.shrink_to_fit();  // 将capacity()减小为与size()相同
c.capacity();       // 不重新分配内存空间，c可以保存多少元素
c.reserve(n);       // 分配至少能容纳n个元素的内存空间
```

## 容器适配器

* stack
* queue
* priority_queue

默认情况下，stakc和queue基于deque实现，priority_queue基于vector实现

priority_queue默认使用<，队列为降序，最高优先级为最大元素；sort默认使用<，结果为升序。

As usual, you can provide the sorting criterion as a template parameter. **By default, the elements are sorted by using operator < in descending order**. Thus, the next element is always the “highest” element.

## 关联容器

map: kev-value

set: key，支持高效的关键字查询操作

* set or map
* 关键字重复或不重复
* 顺序或无序

map multimap unordered_multimap
set multiset unordered_multiset

### 使用关联容器

```c++
// 使用map
// 统计每个单词在输入中出现的次数
map<string, size_t> word_count;
string word;
while(cin >> word) {
    ++word_count[word]; // 如果word还未在map中，下标运算会创建一个新元素，关键字为word，值为0
}
for(const auto &w : word_count) {
    cout << w.first << " occurs " << w.second 
         << ((w.second > 1) ? " times" : " time") << endl;
}
```

```c++
// 使用set
map<string, size_t> word_count;
set<string> exclude = {"The", "But", "And", "Or", "An", "A",
                       "the", "but", "and", "or", "an", "a"};

string word;
while(cin >> word) {
    if(exclude.find(word) == exlucde.end()) {
        ++word_count[word];
    }
}
```

### 关联容器关键字类型的要求

对有序关联容器，关键字类型必须定义元素比较的方法，默认使用 **<**

```c++
bool compareIsbn(const Sales_data &lhs, const Sales_data &rhs) {
    return lhs.isbn() < rhs.isbn();
}

// 当使用decltype来获得一个函数指针类型时，必须加上*
multiset<Sales_data, decltype(compareIsbn) *> bookstore(compareIsbn);
```

### pair类型

```c++
pair<T1, T2> p;     // p是一个pair，两个成员进行了值初始化

pair<T1, T2> p(v1, v2);

pair<T1, T2> p = {v1, v2};  // 等价p(v1, v2)

make_pair(v1, v2)   // 返回一个v1和v2初始化的pair，类型从v1和v2推断出来

```

### 关联容器操作

#### 关联容器迭代器

解引用map迭代器，得到map的value_type的引用，一个pair类型，first成员保存const的关键字，second成员保存值

解引用set迭代器，得到关键字的引用，**且是只读的**

#### 添加元素

```c++
// v是value_type类型的对象，args用来构造一个元素
// 对于map和set，元素的关键字不在c中时才插入或者构造元素
// 函数返回一个pair，包含一个迭代器，指向具有指定关键字的元素，以及一个指示插入是否成功的bool值
c.insert(v);
c.emplace(args);

// b和e是迭代器，表示一个c::value_type类型值的范围；il是这种值的初始化列表
// 函数返回void
c.insert(b, e);
c.insert(il);

// 迭代器p作为一个提示，指出从哪里开始搜索新元素应该存储的位置。返回一个迭代器，指向具有给定关键字的元素
c.insert(p, v);
c.emplace(p, args);
```

对于multiset、multimap，接受单个元素的insert操作返回一个指向新元素的迭代器，无须返回bool值

#### 删除元素

```c++
// 从c中删除关键字为k的元素，返回删除的元素的数量
c.erase(k)

// 删除迭代器p指定的元素，返回一个指向p之后元素的迭代器。若p指向c中的尾元素，返回c.end()
c.erease(p)

// 删除迭代器对b和e所表示的范围中的元素，返回e
c.erase(b, e)
```

#### map下标操作

map和unordered_map的下标操作

```c++
// 返回关键字为k的元素，如果k不在c中，添加关键字为k的元素，进行值初始化
c[k]

// 访问关键字为k的元素，若k不在c中，抛出out_of_range异常
c.at(k)
```

#### 访问元素

在一个关联容器中查找元素的操作

```c++
// 返回一个迭代器，指向第一个关键字为k的元素，若k不在容器中，则返回尾后迭代器
c.find(k)

// 返回关键字等于k的元素的数量
c.count(k)

// 返回一个迭代器，指向第一个关键字不小于k的元素
c.lower_bound(k)

// 返回一个迭代器，指向第一个关键字大于k的元素
c.upper_bound(k)

// 返回迭代器pair，表示关键等于k的元素的范围，若k不存在，pair的两个成员均等于c.end()
c.equal_range(k)
```