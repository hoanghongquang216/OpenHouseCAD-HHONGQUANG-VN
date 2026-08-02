#pragma once

#include <openhouse/model/EntityId.hpp>
#include <openhouse/model/EntityType.hpp>
#include <openhouse/model/PropertySet.hpp>
#include <openhouse/model/EntityStateSnapshot.hpp>
#include <openhouse/model/EntitySnapshot.hpp>
#include <openhouse/model/PropertySnapshot.hpp>

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

    EntityStateSnapshot Snapshot() const
    {
        EntityStateSnapshot snapshot(EntitySnapshot(id_, type_));

        for (const auto& property : properties_.All())
        {
            snapshot.AddProperty(PropertySnapshot(property.Name(), property.Value()));
        }

        return snapshot;
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
