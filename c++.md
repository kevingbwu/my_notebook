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

