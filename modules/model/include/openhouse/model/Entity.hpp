#pragma once

#include <openhouse/model/EntityType.hpp>
#include <openhouse/model/PropertySet.hpp>

namespace openhouse::model
{

class Entity
{
public:
    explicit Entity(EntityType type = EntityType{})
        : type_(type)
    {
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

private:
    EntityType type_;
    PropertySet properties_;
};

}
