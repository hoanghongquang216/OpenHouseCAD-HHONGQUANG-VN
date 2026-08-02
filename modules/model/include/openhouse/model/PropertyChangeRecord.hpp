#pragma once

#include <openhouse/model/EntityId.hpp>

namespace openhouse::model
{

class PropertyChangeRecord
{
public:
    PropertyChangeRecord(
        EntityId entityId,
        PropertyId propertyId)
        : entityId_(entityId),
          propertyId_(propertyId)
    {
    }

    EntityId Entity() const
    {
        return entityId_;
    }

    PropertyId Property() const
    {
        return propertyId_;
    }

private:
    EntityId entityId_;
    PropertyId propertyId_;
};

}
