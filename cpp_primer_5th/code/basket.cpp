#include <bits/stdc++.h>

using namespace std;

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

    // virtual function to return a dynamically allocated copy of itself
    // these members use reference qualifiers; see §13.6.3
    virtual Quote* clone() const &
    {
        return new Quote(*this);
    }
    virtual Quote* clone() &&
    {
        return new Quote(std::move(*this));
    }
private:
    std::string bookNo; // ISBN number of this item
protected:
    double price = 0.0; // normal, undiscounted price
};

class Bulk_quote : public Quote { // Bulk_quote inherits from Quote
public:
    Bulk_quote() = default;
    Bulk_quote(const std::string& book, double p, std::size_t qty, double disc) :
            Quote(book, p), min_qty(qty), discount(disc) { }
    // overrides the base version in order to implement the bulk purchase discount policy
    // if the specified number of items are purchased, use the discounted price
    double net_price(size_t cnt) const override
    {
        if (cnt >= min_qty)
            return cnt * (1 - discount) * price;
        else
            return cnt * price;
    }
    Bulk_quote* clone() const &
    {
        return new Bulk_quote(*this);
    }
    Bulk_quote* clone() &&
    {
        return new Bulk_quote(std::move(*this));
    }
private:
    std::size_t min_qty = 0; // minimum purchase for the discount to apply
    double discount = 0.0; // fractional discount to apply
};

class Basket {
public:
    // Basket uses synthesized default constructor and copy-control members
    void add_item(const std::shared_ptr<Quote> &sale) { items.insert(sale); }
    // 新版本add_item负责内存分配
    void add_item(const Quote& sale) // copy the given object
    { items.insert(std::shared_ptr<Quote>(sale.clone())); }
    void add_item(Quote&& sale) // move the given object
    { items.insert(std::shared_ptr<Quote>(std::move(sale).clone())); }

    // prints the total price for each book and the overall total for all items in the basket
    double total_receipt(std::ostream &) const;

private:
    // function to compare shared_ptrs needed by the multiset member
    static bool compare(const std::shared_ptr<Quote> &lhs,
                        const std::shared_ptr<Quote> &rhs){
        return lhs->isbn() < rhs->isbn();
    }

    // multiset to hold multiple quotes, ordered by the compare member
    std::multiset<std::shared_ptr<Quote>, decltype(compare) *> items{compare};
};

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

double Basket::total_receipt(std::ostream &os) const
{
    double sum = 0.0; // holds the running total
    // iter refers to the first element in a batch of elements with the same ISBN
    // upper_bound returns an iterator to the element just past the end of that batch
    for (auto iter = items.cbegin();
              iter != items.cend();
              iter = items.upper_bound(*iter)) {
        // we know there's at least one element with this key in the Basket
        // print the line item for this book
        sum += print_total(os, **iter, items.count(*iter));
    }
    os << "Total Sale: " << sum << endl; // print the final overall total
    return sum;
}

int main() {
    Basket b;
    Quote q;
    Bulk_quote bq;
    return 0;
}
