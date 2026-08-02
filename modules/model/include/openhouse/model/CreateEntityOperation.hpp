#pragma once

#include <openhouse/model/EntityChangeRecord.hpp>
#include <openhouse/model/EntityId.hpp>
#include <openhouse/model/EntityType.hpp>
#include <openhouse/model/ModelStore.hpp>
#include <openhouse/model/TransactionOperationContext.hpp>

namespace openhouse::model
{

class CreateEntityOperation
{
public:
    CreateEntityOperation(ModelStore& store, EntityType type)
        : store_(store), type_(type)
    {
    }

    EntityId Execute(TransactionOperationContext& context)
    {
        const auto id = store_.Create(type_);

        context.Changes().Add(
            EntityChangeRecord(EntityChangeType::Create, id));

        return id;
    }

private:
    ModelStore& store_;
    EntityType type_;
};

}
