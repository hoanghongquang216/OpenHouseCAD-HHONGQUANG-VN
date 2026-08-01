#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace openhouse::kernel
{

template<class T>
class QueryResult
{
public:
    using Container = std::vector<T*>;
    using Iterator = typename Container::iterator;
    using ConstIterator = typename Container::const_iterator;

    QueryResult() = default;

    explicit QueryResult(Container values)
        : values_(std::move(values))
    {
    }

    std::size_t Size() const
    {
        return values_.size();
    }

    bool Empty() const
    {
        return values_.empty();
    }

    Iterator begin()
    {
        return values_.begin();
    }

    Iterator end()
    {
        return values_.end();
    }

    ConstIterator begin() const
    {
        return values_.begin();
    }

    ConstIterator end() const
    {
        return values_.end();
    }

    const T* operator[](std::size_t index) const
    {
        return values_[index];
    }

private:
    Container values_;
};

}
