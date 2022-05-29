#include <bits/stdc++.h>

using namespace std;

// simplified implementation of the memory allocation strategy for a vector-like class
class StrVec {
public:
    StrVec(): // the allocator member is default initialized
        elements(nullptr), first_free(nullptr), cap(nullptr) {
    }
    StrVec(const StrVec&); // copy constructor
    StrVec &operator=(const StrVec&); // copy assignment
    ~StrVec(); // destructor
    void push_back(const std::string&); // copy the element
    size_t size() const { return first_free - elements; }
    size_t capacity() const { return cap - elements; }
    std::string *begin() const { return elements; }
    std::string *end() const { return first_free; }
    // ...
private:
    std::allocator<std::string> alloc; // allocates the elements
    // used by the functions that add elements to the StrVec
    void chk_n_alloc() {
        if (size() == capacity()) reallocate();
    }
    // utilities used by the copy constructor, assignment operator, and destructor
    // 分配内存，拷贝给定范围中的元素
    std::pair<std::string*, std::string*> alloc_n_copy(const std::string*, const std::string*);
    void free(); // destroy the elements and free the space
    void reallocate(); // get more space and copy the existing elements

    std::string *elements; // pointer to the first element in the array
    std::string *first_free; // pointer to the first free element in the array
    std::string *cap; // pointer to one past the end of the array
};

void StrVec::push_back(const string& s)
{
    chk_n_alloc(); // ensure that there is room for another element
    // construct a copy of s in the element to which first_free points
    alloc.construct(first_free++, s);
}

pair<string*, string*>
StrVec::alloc_n_copy(const string *b, const string *e)
{
    // allocate space to hold as many elements as are in the range
    auto data = alloc.allocate(e - b);
    // initialize and return a pair constructed from data and
    // the value returned by uninitialized_copy
    return {data, uninitialized_copy(b, e, data)};
}

void StrVec::free()
{
    // may not pass deallocate a 0 pointer; if elements is 0, there's no work to do
    if (elements) {
    // destroy the old elements in reverse order
        for (auto p = first_free; p != elements; /* empty */)
            alloc.destroy(--p);
        alloc.deallocate(elements, cap - elements);
    }
}

StrVec::StrVec(const StrVec &s)
{
    // call alloc_n_copy to allocate exactly as many elements as in s
    auto newdata = alloc_n_copy(s.begin(), s.end());
    elements = newdata.first;
    first_free = cap = newdata.second;
}

StrVec::~StrVec() { free(); }

StrVec &StrVec::operator=(const StrVec &rhs)
{
    // call alloc_n_copy to allocate exactly as many elements as in rhs
    auto data = alloc_n_copy(rhs.begin(), rhs.end());
    free();
    elements = data.first;
    first_free = cap = data.second;
    return *this;
}

void StrVec::reallocate()
{
    // we'll allocate space for twice as many elements as the current size
    auto newcapacity = size() ? 2 * size() : 1;
    // allocate new memory
    auto newdata = alloc.allocate(newcapacity);
    // move the data from the old memory to the new
    auto dest = newdata; // points to the next free position in the new array
    auto elem = elements; // points to the next element in the old array
    for (size_t i = 0; i != size(); ++i)
        alloc.construct(dest++, std::move(*elem++));
    free(); // free the old space once we've moved the elements
    // update our data structure to point to the new elements
    elements = newdata;
    first_free = dest;
    cap = elements + newcapacity;
}

int main() {
    StrVec svec;
    svec.push_back("abc");
    cout << svec.capacity() << endl;
    svec.push_back("def");
    cout << svec.capacity() << endl;
    svec.push_back("ghi");
    cout << svec.capacity() << endl;
    for (auto p = svec.begin(); p != svec.end(); ++p) {
        cout << *p << endl;
    }
    return 0;
}