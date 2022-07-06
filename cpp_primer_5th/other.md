# 其他主题

## 命名空间

命名空间分割了全局命名空间，每个命名空间是一个作用域

### 命名空间定义

```c++
namespace cplusplus_primer {
    class Sales_data { / * ... * /};
    Sales_data operator+(const Sales_data&, const Sales_data&);
    class Query { /* ... */ };
    class Query_base { /* ... */};
} // like blocks, namespaces do not end with a semicolon
```

命名空间不能定义在函数或类的内部

#### 每个命名空间都是一个作用域

位于命名空间之外的代码必须明确指出所用的名字属于哪个命名空间

#### 命名空间可以是不连续的

```c++
namespace nsp {
    // 相关声明
}
```

可能定义了一个名为nsp的新命名空间，也可能为已经存在的命名空间添加一些新成员

#### 模板特例化

模板特例化必须定义在原始模板所属的命名空间中。在命名空间中声明了特例化，就能在命名空间外部定义它了

```c++
// we must declare the specialization as a member of std
namespace std {
    template <> struct hash<Sales_data>;
}

// having added the declaration for the specialization to std
// we can define the specialization outside the std namespace
template <> struct std::hash<Sales_data> {
    size_t operator()(const Sales_data& s) const {
        return hash<string>()(s.bookNo) ^
               hash<unsigned>()(s.units_sold) ^
               hash<double>()(s.revenue);
    }
    // other members as before
};
```

#### 全局命名空间

全局作用域中定义的名字被隐式地添加到全局命名空间中

`::member_name`

#### 嵌套的命名空间

```c++
namespace cplusplus_primer {
    // first nested namespace: defines the Query portion of the library
    namespace QueryLib {
        class Query { /* ... */ };
        Query operator&(const Query&, const Query&);
        // ...
    }
    // second nested namespace: defines the Sales_data portion of the library
    namespace Bookstore {
        class Quote { /* ... */ };
        class Disc_quote : public Quote { /* ... */ };
        // ...
    }
}
```

#### 内联命名空间

> C++11：内联命名空间

内联命名空间中的名字可以被外层命名空间直接使用

```c++
inline namespace FifthEd {
    // namespace for the code from the Primer Fifth Edition
}

namespace FifthEd { // implicitly inline
    class Query_base { /* ... */ };
    // other Query-related declarations
}

namespace FourthEd {
    class Item_base { /* ... */};
    class Query_base { /* ... */};
    // other code from the Fourth Edition
}

namespace cplusplus_primer {
#include "FifthEd.h"
#include "FourthEd.h"
}

// cplusplus_primer::的代码直接获得FifthEd的成员
```

#### 未命名的命名空间

每个文件定义自己的未命名的命名空间

```c++
int i; // global declaration for i

namespace {
    int i;
}

// ambiguous: defined globally and in an unnested, unnamed namespace
i = 10;

namespace local {
    namespace {
        int i;
    }
}
// ok: i defined in a nested unnamed namespace is distinct from global i
local::i = 42;
```

未命名的命名空间取代文件中的静态声明