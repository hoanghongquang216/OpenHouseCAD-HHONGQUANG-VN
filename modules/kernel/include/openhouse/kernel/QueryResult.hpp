#pragma once

#include <cstddef>
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

private:
    Container values_;
};

}
