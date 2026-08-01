#pragma once

#include <vector>

namespace openhouse::kernel
{

template<class T, class Store>
std::vector<T*> FindAll(Store& store)
{
    std::vector<T*> result;

    for (auto& item : store)
    {
        if (auto* object = dynamic_cast<T*>(item.second.get()))
        {
            result.push_back(object);
        }
    }

    return result;
}

template<class T, class Store>
std::vector<const T*> FindAll(const Store& store)
{
    std::vector<const T*> result;

    for (const auto& item : store)
    {
        if (auto* object = dynamic_cast<const T*>(item.second.get()))
        {
            result.push_back(object);
        }
    }

    return result;
}

}
