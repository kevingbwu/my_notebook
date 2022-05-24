# 泛型算法

标准库并未给每个容器添加大量功能，而是提供了一组算法，独立于任何特定容器。这些算法是通用的或泛型的(generic)

## 概述

大多数算法定义在头文件algorithm，数值泛型算法在头文件numberic

算法并不直接操作容器，而是遍历由两个迭代器指定的一个元素范围来进行操作

```c++
int val = 42; // value we'll look for
// result will denote the element we want if it's in vec, or vec.cend() if not
auto result = find(vec.cbegin(), vec.cend(), val);
// report the result
cout << "The value " << val
     << (result == vec.cend()
     ? " is not present" : " is present") << endl;

string val = "a value"; // value we'll look for
// this call to find looks through string elements in a list
auto result = find(lst.cbegin(), lst.cend(), val);

int ia[] = {27, 210, 12, 47, 109, 83};
int val = 83;
int* result = find(begin(ia), end(ia), val);

// search the elements starting from ia[1] up to but not including ia[4]
auto result = find(ia + 1, ia + 4, val);
```

**迭代器令算法不依赖于容器，但算法依赖于元素类型的操作**

**算法不会改变底层容器的大小**

**操作特殊的迭代器——插入器(inserter)在底层容器上执行插入操作**

## 使用泛型算法

是否读取元素、改变元素、重拍元素顺序

### 只读算法

* find
* count
* accumulate：第三个参数的类型决定了函数中使用哪个加法运算符以及返回值的类型

```c++
// sum the elements in vec starting the summation with the value 0
int sum = accumulate(vec.cbegin(), vec.cend(), 0);

string sum = accumulate(v.cbegin(), v.cend(), string(""));

// error: no + on const char*
string sum = accumulate(v.cbegin(), v.cend(), "");
```

* equal

```c++
// roster2 should have at least as many elements as roster1
equal(roster1.cbegin(), roster1.cend(), roster2.cbegin());
```

**接受单一迭代器来表示第二个序列的算法都假定第二个序列至少与第一个序列一样长**

### 写容器元素的算法

* fill

```c++
fill(vec.begin(), vec.end(), 0); // reset each element to 0
// set a subsequence of the container to 10
fill(vec.begin(), vec.begin() + vec.size()/2, 10);
```

* fill_n

```c++
vector<int> vec; // empty vector
// use vec giving it various values
fill_n(vec.begin(), vec.size(), 0); // reset all the elements of vec to 0
// disaster: attempts to write to ten (nonexistent) elements in vec
fill_n(vec.begin(), 10, 0);
```

#### back_inserter

通过一个插入迭代器赋值时，与赋值号右侧值相等的元素被添加到容器中。

back_inserter接受一个指向容器的引用，返回一个与该容器绑定的插入迭代器

```c++
vector<int> vec; // empty vector
auto it = back_inserter(vec); // assigning through it adds elements to vec
*it = 42; // vec now has one element with value 42

vector<int> vec; // empty vector
// ok: back_inserter creates an insert iterator that adds elements to vec
fill_n(back_inserter(vec), 10, 0); // appends ten elements to vec
```

* copy：返回目的位置迭代器的值

```c++
int a1[] = {0,1,2,3,4,5,6,7,8,9};
int a2[sizeof(a1)/sizeof(*a1)]; // a2 has the same size as a1
// ret points just past the last element copied into a2
auto ret = copy(begin(a1), end(a1), a2); // copy a1 into a2
```

* replace：读取一个序列，将其中所有等于给定值的元素都改为另一个值

```c++
// replace any element with the value 0 with 42
replace(ilst.begin(), ilst.end(), 0, 42);
```

* replace_copy

```c++
// use back_inserter to grow destination as needed
replace_copy(ilst.cbegin(), ilst.cend(), back_inserter(ivec), 0, 42);
```

### 重排元素的算法

* sort

* unique

```c++
void elimDups(vector<string> &words)
{
    // sort words alphabetically so we can find the duplicates
    sort(words.begin(), words.end());
    // unique reorders the input range so that each word appears once in the
    // front portion of the range and returns an iterator one past the unique range
    auto end_unique = unique(words.begin(), words.end());
    // erase uses a vector operation to remove the nonunique elements
    words.erase(end_unique, words.end());
}
```

## 定制操作

### 向算法传递函数

```c++
// comparison function to be used to sort by word length
bool isShorter(const string &s1, const string &s2)
{
    return s1.size() < s2.size();
}
// sort on word length, shortest to longest
sort(words.begin(), words.end(), isShorter);
```

* stable_sort

```c++
elimDups(words); // put words in alphabetical order and remove duplicates
// resort by length, maintaining alphabetical order among words of the same length
stable_sort(words.begin(), words.end(), isShorter);
for (const auto &s : words) // no need to copy the strings
    cout << s << " "; // print each element separated by a space
cout << endl;
```

### lambda表达式

可调用对象：

* 函数
* 函数指针
* 重载了函数调用运算符的类
* lambda表达式：可调用的代码单元，未命名内联函数

`[capture list] (parameter list) -> return type { function body }`

可以忽略参数列表和返回类型，必须包含捕获列表和函数体

```c++
auto f = [] { return 42; };
cout << f() << endl; // prints 42
```

#### 向lambda传递参数

lambda不能有默认参数

```c++
// sort words by size, but maintain alphabetical order for words of the same size
stable_sort(words.begin(), words.end(), [](const string &a, const string &b) { 
    return a.size() < b.size();
});
```

#### 使用捕获列表

捕获列表只用于局部非static变量，可以直接使用局部static变量和它所在函数之外声明的名字

```c++
[sz](const string &a)
{ return a.size() >= sz; };

// error: sz not captured
[](const string &a)
{ return a.size() >= sz; };
```

* find_if

```c++
// get an iterator to the first element whose size() is >= sz
auto wc = find_if(words.begin(), words.end(), [sz](const string &a) { 
    return a.size() >= sz;
});

// compute the number of elements with size >= sz
auto count = words.end() - wc;
cout << count << " " << make_plural(count, "word", "s")
     << " of length " << sz << " or longer" << endl;
```

* for_each

```c++
// print words of the given size or longer, each one followed by a space
for_each(wc, words.end(), [](const string &s) {
    cout << s << " ";
});
cout << endl;
```

### lambda捕获和返回

定义一个lambda时，编译器生成一个与lambda对应的新的未命名类类型。向一个函数传递一个lambda时，同时定义了一个新类型和该类型的一个对象，传递的参数就是此编译器生成的类类型的未命名对象。当使用auto定义一个用lambda初始化的变量时，定义了一个从lambda生成的类型的对象

#### 值捕获

采用值捕获的前提是变量可拷贝，被捕获的变量是在lambda创建时被拷贝，而不是调用时拷贝

```c++
void fcn1()
{
    size_t v1 = 42; // local variable
    // copies v1 into the callable object named f
    auto f = [v1] { return v1; };
    v1 = 0;
    auto j = f(); // j is 42; f stored a copy of v1 when we created it
}
```

#### 引用捕获

```c++
void fcn2()
{
    size_t v1 = 42; // local variable
    // the object f2 contains a reference to v1
    auto f2 = [&v1] { return v1; };
    v1 = 0;
    auto j = f2(); // j is 0; f2 refers to v1; it doesn't store it
}
```

采用引用方式捕获一个变量，必须确保被引用的对象在lambda执行的时候是存在的

如果对象不能拷贝，则只能捕获其引用

```c++
void biggies(vector<string> &words,
vector<string>::size_type sz,
ostream &os = cout, char c = ' ')
{
    // code to reorder words as before
    // statement to print count revised to print to os
    for_each(words.begin(), words.end(),
    [&os, c](const string &s) { os << s << c; });
}
```

可以从一个函数返回lambda，此lambda不能包含引用捕获

#### 隐式捕获

在捕获列表中写一个&或=，&告诉编译器采用捕获引用方式，=表示采用值捕获方式

```c++
// sz implicitly captured by value
wc = find_if(words.begin(), words.end(), [=](const string &s) { 
    return s.size() >= sz; 
});
```

混合使用隐式捕获和显示捕获

```c++
void biggies(vector<string> &words,
vector<string>::size_type sz,
ostream &os = cout, char c = ' ')
{
    // other processing as before
    // os implicitly captured by reference; c explicitly captured by value
    for_each(words.begin(), words.end(),
    [&, c](const string &s) { os << s << c; });
    // os explicitly captured by reference; c implicitly captured by value
    for_each(words.begin(), words.end(),
    [=, &os](const string &s) { os << s << c; });
}
```

#### 可变lambda

默认情况下，对一个值被拷贝的变量，lambda不会改变其值，如果希望能改变，在参数列表后加上关键字mutable

```c++
void fcn3()
{
    size_t v1 = 42; // local variable
    // f can change the value of the variables it captures
    auto f = [v1] () mutable { return ++v1; };
    v1 = 0;
    auto j = f(); // j is 43
}

void fcn4()
{
    size_t v1 = 42; // local variable
    // v1 is a reference to a non const variable
    // we can change that variable through the reference inside f2
    auto f2 = [&v1] { return ++v1; };
    v1 = 0;
    auto j = f2(); // j is 1
}
```

#### 指定lambda返回类型

如果一个lambda体内包含return之外的任何语句，则编译器假定此lambda返回void

```c++
transform(vi.begin(), vi.end(), vi.begin(),
[](int i) { return i < 0 ? -i : i; });

// error: cannot deduce the return type for the lambda
transform(vi.begin(), vi.end(), vi.begin(), [](int i) { 
    if (i < 0) return -i; else return i;
});

transform(vi.begin(), vi.end(), vi.begin(), [](int i) -> int { 
    if (i < 0) return -i; else return i; 
});
```

### 参数绑定

只在一两个地方使用的简单操作，lambda表达式是最有用的；如果需要在很多地方使用相同的操作，通常应该定义一个函数，而不是多次编写相同的lambda表达式

```c++
// find_if接受一元谓词，如何用check_size代替lambda
bool check_size(const string &s, string::size_type sz)
{
    return s.size() >= sz;
}
```

#### 标准库bind函数

> C++11：通用的函数适配器，接受一个可调用对象，生成一个新的可调用对象来适应原对象的参数列表，定义在头文件functional中

`auto newCallable = bind(callable, arg_list);`

_n：占位符，标识newCallable的参数，占了局传递给newCallable的参数的位置，位于命名空间`std::placeholders`

```c++
// check6 is a callable object that takes one argument of type string
// and calls check_size on its given string and the value 6
auto check6 = bind(check_size, _1, 6);

string s = "hello";
bool b1 = check6(s); // check6(s) calls check_size(s, 6)

auto wc = find_if(words.begin(), words.end(), [sz](const string &a) { 
    return a.size() >= sz;
});
// 替换为
auto wc = find_if(words.begin(), words.end(),
bind(check_size, _1, sz));
```

#### bind的参数

重排参数顺序

```c++
// g is a callable object that takes two arguments
auto g = bind(f, a, b, _2, c, _1);

// call g
g(_1, _2)
// to f
f(a, b, _2, c, _1)

// sort on word length, shortest to longest
sort(words.begin(), words.end(), isShorter);
// sort on word length, longest to shortest
sort(words.begin(), words.end(), bind(isShorter, _2, _1));
```

#### 绑定引用参数

```c++
// os is a local variable referring to an output stream
// c is a local variable of type char
for_each(words.begin(), words.end(),
[&os, c](const string &s) { os << s << c; });

// 等价函数
ostream &print(ostream &os, const string &s, char c)
{
    return os << s << c;
}

// error: cannot copy os
for_each(words.begin(), words.end(), bind(print, os, _1, ' '));

// ref返回一个对象，包含给定的引用
for_each(words.begin(), words.end(), bind(print, ref(os), _1, ' '));
```

## 迭代器

* 每个容器自己定义的迭代器
* 插入迭代器
* 流迭代器
* 反向迭代器
* 移动迭代器

### 插入迭代器

接受一个容器，生成一个迭代器，能够实现向给定容器添加元素

* back_inserter
* front_inserter
* inserter

```c++
// it is an iterator generated by inserter
*it = va1;
// 等价于
it = c.insert(it, val); // it points to the newly added element
++it; // increment it so that it denotes the same element as before

list<int> lst = {1,2,3,4};
list<int> lst2, lst3; // empty lists

// after copy completes, 1st2 contains 4 3 2 1
copy(lst.cbegin(), lst.cend(), front_inserter(lst2));
// after copy completes, 1st3 contains 1 2 3 4
copy(lst.cbegin(), lst.cend(), inserter(lst3, lst3.begin()));
```

### iostream迭代器

通过使用流迭代器，可以使用泛型算法从流对象读取数据以及向其写入数据

#### istream_iterator操作

```c++
istream_iterator<int> int_it(cin); // reads ints from cin
istream_iterator<int> int_eof; // end iterator value
ifstream in("afile");
istream_iterator<string> str_it(in); // reads strings from "afile"

istream_iterator<int> in_iter(cin); // read ints from cin
istream_iterator<int> eof; // istream ''end'' iterator
while (in_iter != eof) // while there's valid input to read
    // postfix increment reads the stream and returns the old value of the iterator
    // we dereference that iterator to get the previous value read from the stream
    vec.push_back(*in_iter++);

// 等价于
istream_iterator<int> in_iter(cin), eof; // read ints from cin
vector<int> vec(in_iter, eof); // construct vec from an iterator range

// 算法操作流迭代器
istream_iterator<int> in(cin), eof;
cout << accumulate(in, eof, 0) << endl;
```

#### ostream_iterator操作

```c++
ostream_iterator<int> out_iter(cout, " ");
for (auto e : vec)
    *out_iter++ = e; // the assignment writes this element to cout
cout << endl;
// *和++其实不做任何事
for (auto e : vec)
    out_iter = e; // the assignment writes this element to cout
cout << endl;
// 等价于
copy(vec.begin(), vec.end(), out_iter);
cout << endl;
```

### 反向迭代器

除了forward_list之外，其他容器都支持反向迭代器

```c++
vector<int> vec = {0,1,2,3,4,5,6,7,8,9};
// reverse iterator of vector from back to front
for (auto r_iter = vec.crbegin(); // binds r_iter to the last element
          r_iter != vec.crend(); // crend refers 1 before 1st element
          ++r_iter) // decrements the iterator one element
    cout << *r_iter << endl; // prints 9, 8, 7,... 0

sort(vec.begin(), vec.end()); // sorts vec in ''normal'' order
// sorts in reverse: puts the smallest element at the end of vec
sort(vec.rbegin(), vec.rend());

// find the first element in a comma-separated list
auto comma = find(line.cbegin(), line.cend(), ',');
cout << string(line.cbegin(), comma) << endl;

// find the last element in a comma-separated list
auto rcomma = find(line.crbegin(), line.crend(), ',');

// WRONG: will generate the word in reverse order
cout << string(line.crbegin(), rcomma) << endl;

// ok: get a forward iterator and read to the end of line
cout << string(rcomma.base(), line.cend()) << endl;
```

## 泛型算法结构

迭代器类别：
* 输入迭代器
* 输出迭代器
* 前向迭代器
* 双向迭代器
* 随机访问迭代器

## 特定容器算法

list和forward_list定义了几个成员函数形式的算法：

* merge
* remove
* reverse
* sort
* unique
* splice
* splice_after