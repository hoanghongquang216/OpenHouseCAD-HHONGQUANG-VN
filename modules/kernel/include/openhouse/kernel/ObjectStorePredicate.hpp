#pragma once

#include <vector>

namespace openhouse::kernel
{

template<class T, class Store, class Predicate>
std::vector<T*> FindIf(Store& store, Predicate&& predicate)
{
    std::vector<T*> result;

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

    return result;
}

}
