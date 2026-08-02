#pragma once

#include <vector>

#include <openhouse/model/EntitySnapshot.hpp>
#include <openhouse/model/PropertySnapshot.hpp>

namespace openhouse::model
{

class EntityStateSnapshot
{
public:
    explicit EntityStateSnapshot(EntitySnapshot entity)
        : entity_(std::move(entity))
    {
    }

    void AddProperty(PropertySnapshot property)
    {
        properties_.push_back(std::move(property));
    }

    const EntitySnapshot& Entity() const
    {
        return entity_;
    }

    const std::vector<PropertySnapshot>& Properties() const
    {
        return properties_;
    }

private:
    EntitySnapshot entity_;
    std::vector<PropertySnapshot> properties_;
};

}
