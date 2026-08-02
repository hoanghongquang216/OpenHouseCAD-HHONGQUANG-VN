#pragma once

#include <unordered_map>

#include <openhouse/model/Entity.hpp>

namespace openhouse::model
{

class EntityRegistry
{
public:
    bool Add(Entity entity)
    {
        const auto id = entity.Id().Value();

        if (id == 0 || entities_.contains(id))
        {
            return false;
        }

        entities_.emplace(id, std::move(entity));
        return true;
    }

    bool Remove(EntityId id)
    {
        return entities_.erase(id.Value()) > 0;
    }

    Entity* Find(EntityId id)
    {
        auto it = entities_.find(id.Value());

        if (it == entities_.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    const Entity* Find(EntityId id) const
    {
        auto it = entities_.find(id.Value());

        if (it == entities_.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    std::size_t Count() const
    {
        return entities_.size();
    }

private:
    std::unordered_map<EntityIdValue, Entity> entities_;
};

}
