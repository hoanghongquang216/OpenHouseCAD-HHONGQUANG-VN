#pragma once

#include <cstdint>

#include <openhouse/model/EntityRegistry.hpp>
#include <openhouse/model/EntityStateSnapshot.hpp>

namespace openhouse::model
{

class ModelStore
{
public:
    explicit ModelStore(EntityIdValue nextId = 1)
        : nextId_(nextId)
    {
    }

    EntityId Create(EntityType type)
    {
        EntityId id(nextId_++);
        registry_.Add(Entity(type, id));
        return id;
    }

    bool Remove(EntityId id)
    {
        return false;
    }

    bool Restore(const EntityStateSnapshot& snapshot)
    {
        Entity* entity = registry_.Find(snapshot.Entity().Id());

        if (entity == nullptr)
        {
            return false;
        }

        entity->Restore(snapshot);
        return true;
    }

    Entity* Find(EntityId id)
    {
        return registry_.Find(id);
    }

    const Entity* Find(EntityId id) const
    {
        return registry_.Find(id);
    }

    std::size_t Count() const
    {
        return registry_.Count();
    }

private:
    EntityIdValue nextId_;
    EntityRegistry registry_;
};

}
