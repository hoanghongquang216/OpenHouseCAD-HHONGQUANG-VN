#pragma once

#include <cstddef>

namespace openhouse::kernel
{

template<class Store, class Function>
void ForEach(Store& store, Function&& function)
{
    for (auto& item : store)
    {
        function(item.second.get());
    }
}

template<class Store, class Function>
void ForEach(const Store& store, Function&& function)
{
    for (const auto& item : store)
    {
        function(item.second.get());
    }
}

}
