#pragma once

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <openhouse/kernel/Entity.hpp>
#include <openhouse/kernel/Handle.hpp>
#include <openhouse/kernel/ObjectId.hpp>
#include <openhouse/kernel/ObjectIdGenerator.hpp>

namespace openhouse::kernel
{

class ObjectStore
{
public:
    ObjectStore() = default;

    template<class T, class... Args>
    T* Create(Args&&... args)
    {
        return Create<T>(idGenerator_.Next(), std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    T* Create(ObjectId id, Args&&... args)
    {
        static_assert(std::is_base_of_v<Entity, T>);

        if (Contains(id))
        {
            throw std::runtime_error("ObjectId already exists");
        }

        auto object = std::make_unique<T>(id, std::forward<Args>(args)...);
        auto pointer = object.get();
        objects_.emplace(id, std::move(object));
        return pointer;
    }

    template<class T>
    Handle<T> Get(ObjectId id) const
    {
        auto* entity = Find(id);
        return Handle<T>(dynamic_cast<T*>(entity));
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
    ObjectIdGenerator idGenerator_;
    std::unordered_map<ObjectId, std::unique_ptr<Entity>> objects_;
};

}
