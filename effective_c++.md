# Effective C++

## 1. Accustomming Yourself to C++

### 01 View C++ as a federation of languages

* C
* Object-Oriented C++
* Template C++
* STL

### 02 Prefer consts, enums, inline to #define

宁可以编译器替换预处理器

\#define 不重视作用域，没有封装性

```c++
class GamePlayer {
private:
    static const int NumTurns = 5;  // 常量声明
    int scores[NumTurns];           // 使用常量
}

class GamePlayer {
private:
    enum { NumTurns = 5 };  // the enum hack
    int scores[NumTurns];   // NumTurns成为5的一个记号名称
}
```

最好用inline函数替换函数式宏

### 03 User const whenever possible

### 04 Make sure that objects are initialized before they're used

## 2. Constructors, Destructors, and Assignment Operators

### 05 Know what functions C++ silently writes and calls