#pragma once

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <openhouse/kernel/ObjectId.hpp>

namespace openhouse::kernel
{

class Entity;

template<class T>
class Handle;

class ObjectStore
{
public:
    ObjectStore() = default;

    template<class T, class... Args>
    T* Create(ObjectId id, Args&&... args)
    {
        if (Contains(id))
        {
            throw std::runtime_error("ObjectId already exists");
        }

        auto object = std::make_unique<T>(id, std::forward<Args>(args)...);
        auto pointer = object.get();
        objects_.emplace(id, std::move(object));
        return pointer;
    }

    bool Contains(ObjectId id) const
    {
        return objects_.find(id) != objects_.end();
    }

    Entity* Find(ObjectId id) const
    {
        auto iterator = objects_.find(id);
        return iterator == objects_.end() ? nullptr : iterator->second.get();
    }

    std::size_t Size() const
    {
        return objects_.size();
    }

private:
    std::unordered_map<ObjectId, std::unique_ptr<Entity>> objects_;
};

}
