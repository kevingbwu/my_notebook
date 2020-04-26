# C++ Notes

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

## 访问元素

```c++
c.back();   // 返回c中尾元素的引用
c.front();  // 返回c中头元素的引用

// at和下标操作只适用于string、vector、deque、array
c[n];       // 返回c中下标为n的元素的引用
c.at(n);    // 返回c中下标为n的元素的引用
```

## 删除元素

```c++
c.pop_back();   // 删除c中的尾元素
c.pop_front();  // 删除c中的头元素，vector和string不支持
c.erase(p);     // 删除迭代器p所指向的元素，返回被删元素之后的迭代器
c.erase(b, e);  // 删除迭代器b和e所指定范围内的元素
c.clear();      // 删除c中所有元素
```

## 改变容器大小

```c++
c.resize(n);    // 调整c的大小为n个元素
c.resize(n, t); // 调整c的大小为n个元素，新添加的元素初始化为值t
```

## 容器操作可能使迭代器失效

## vector对象如何增长

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