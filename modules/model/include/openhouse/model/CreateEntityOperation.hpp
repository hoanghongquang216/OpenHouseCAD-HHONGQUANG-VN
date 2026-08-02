#pragma once

#include <openhouse/model/EntityId.hpp>
#include <openhouse/model/EntityType.hpp>
#include <openhouse/model/ModelStore.hpp>

namespace openhouse::model
{

class CreateEntityOperation
{
public:
    CreateEntityOperation(ModelStore& store, EntityType type)
        : store_(store), type_(type)
    {
    }

    EntityId Execute()
    {
        return store_.Create(type_);
    }

private:
    ModelStore& store_;
    EntityType type_;
};

}
