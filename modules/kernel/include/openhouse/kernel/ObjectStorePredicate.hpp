#pragma once

#include <openhouse/kernel/QueryResult.hpp>

namespace openhouse::kernel
{

template<class T, class Store, class Predicate>
QueryResult<T> FindIf(Store& store, Predicate&& predicate)
{
    typename QueryResult<T>::Container result;

    for (auto& item : store)
    {
        if (auto* object = dynamic_cast<T*>(item.second.get()))
        {
            if (predicate(*object))
            {
                result.push_back(object);
            }
        }
    }

    return QueryResult<T>(std::move(result));
}

}
