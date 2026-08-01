#pragma once

#include <openhouse/kernel/QueryResult.hpp>

namespace openhouse::kernel
{

template<class T, class Store>
QueryResult<T> FindAll(Store& store)
{
    typename QueryResult<T>::Container result;

    for (auto& item : store)
    {
        if (auto* object = dynamic_cast<T*>(item.second.get()))
        {
            result.push_back(object);
        }
    }

    return QueryResult<T>(std::move(result));
}

template<class T, class Store>
QueryResult<const T> FindAll(const Store& store)
{
    typename QueryResult<const T>::Container result;

    for (const auto& item : store)
    {
        if (auto* object = dynamic_cast<const T*>(item.second.get()))
        {
            result.push_back(object);
        }
    }

    return QueryResult<const T>(std::move(result));
}

}
