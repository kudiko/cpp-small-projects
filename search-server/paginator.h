#pragma once

#include <vector>
#include <iostream>

template <typename Iterator>
class IteratorRange
{
public:
    IteratorRange(Iterator range_begin, Iterator range_end) : begin_(range_begin), end_(range_end) {}
    Iterator begin() const
    {
        return begin_;
    }
    Iterator end() const
    {
        return end_;
    }
    std::size_t size() const
    {
        return std::distance(begin_, end_);
    }

private:
    const Iterator begin_;
    const Iterator end_;
};

template <typename Iterator>
class Paginator
{
public:
    Paginator(Iterator begin, Iterator end, std::size_t page_size) : page_size_(page_size)
    {
        auto left_it = begin;
        for (auto doc_it = left_it; doc_it != end; ++doc_it)
        {
            if (std::distance(left_it, doc_it) % page_size_ == 0 && doc_it != begin)
            {
                page_ranges_.push_back({left_it, doc_it});
                left_it = doc_it;
            }
        }
        page_ranges_.push_back({left_it, end});
    }
    auto begin() const
    {
        return page_ranges_.begin();
    }
    auto end() const
    {
        return page_ranges_.end();
    }

private:
    const std::size_t page_size_;
    std::vector<IteratorRange<Iterator>> page_ranges_;
};

template <typename Iterator>
std::ostream &operator<<(std::ostream &out, const IteratorRange<Iterator> &iter_range)
{
    for (auto it = iter_range.begin(); it != iter_range.end(); ++it)
    {
        out << *it;
    }
    return out;
}

template <typename Container>
auto Paginate(const Container &c, std::size_t page_size)
{
    return Paginator(begin(c), end(c), page_size);
}