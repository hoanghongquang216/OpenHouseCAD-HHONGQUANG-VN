#pragma once

#include <openhouse/model/ModelStore.hpp>
#include <openhouse/model/ModelTransaction.hpp>

namespace openhouse::model
{

class TransactionalModelStore
{
public:
    explicit TransactionalModelStore(ModelStore& store)
        : store_(store)
    {
    }

    void Begin()
    {
        transaction_.Begin();
    }

    EntityId Create(EntityType type)
    {
        EntityId id = store_.Create(type);
        transaction_.Record(TransactionChange(ChangeOperation::Create, id));
        return id;
    }

    void Commit()
    {
        transaction_.Commit();
    }

    void Rollback()
    {
        transaction_.Rollback();
    }

    bool IsActive() const
    {
        return transaction_.IsActive();
    }

private:
    ModelStore& store_;
    ModelTransaction transaction_;
};

}
