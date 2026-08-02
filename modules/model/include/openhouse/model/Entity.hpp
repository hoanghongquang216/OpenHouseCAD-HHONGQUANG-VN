#pragma once

#include <openhouse/model/EntityId.hpp>
#include <openhouse/model/EntityType.hpp>
#include <openhouse/model/PropertySet.hpp>
#include <openhouse/model/EntityStateSnapshot.hpp>

namespace openhouse::model
{

class Entity
{
public:
    explicit Entity(EntityType type = EntityType{}, EntityId id = EntityId{})
        : id_(id), type_(type)
    {
    }

    EntityId Id() const
    {
        return id_;
    }

    EntityType Type() const
    {
        return type_;
    }

    PropertySet& Properties()
    {
        return properties_;
    }

    const PropertySet& Properties() const
    {
        return properties_;
    }

    void Restore(const EntityStateSnapshot& snapshot)
    {
        properties_.RestoreSnapshots(snapshot.Properties());
    }

private:
    EntityId id_;
    EntityType type_;
    PropertySet properties_;
};

}
