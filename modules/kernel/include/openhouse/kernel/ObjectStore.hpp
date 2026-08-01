#pragma once

#include <memory>
#include <unordered_map>

#include <openhouse/kernel/ObjectId.hpp>

namespace openhouse::kernel
{

class Entity;

class ObjectStore
{
public:
    ObjectStore() = default;

    template<class T, class... Args>
    T* Create(ObjectId id, Args&&... args)
    {
        auto object = std::make_unique<T>(id, std::forward<Args>(args)...);
        auto pointer = object.get();
        objects_.emplace(id, std::move(object));
        return pointer;
    }

    Entity* Find(ObjectId id) const
    {
        auto iterator = objects_.find(id);
        return iterator == objects_.end() ? nullptr : iterator->second.get();
    }

private:
    std::unordered_map<ObjectId, std::unique_ptr<Entity>> objects_;
};

}
