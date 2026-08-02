#include <cassert>

#include <openhouse/model/HistoryManager.hpp>
#include <openhouse/model/TransactionExecutor.hpp>
#include <openhouse/model/PropertySnapshot.hpp>
#include <openhouse/model/EntityStateSnapshot.hpp>
#include <openhouse/model/EntitySnapshot.hpp>
#include <openhouse/model/TransactionChange.hpp>

using namespace openhouse::model;

int main()
{
    HistoryManager history;

    assert(!history.CanUndo());
    assert(!history.CanRedo());

    ModelStore store;
    EntityId id = store.Create(EntityType{});

    Entity* entity = store.Find(id);
    assert(entity != nullptr);

    entity->Properties().Add(Property("Height", "3000"));

    EntityStateSnapshot before = entity->Snapshot();

    entity->Properties().All()[0].SetValue("3500");

    EntityStateSnapshot after = entity->Snapshot();

    TransactionChange change(
        ChangeOperation::Modify,
        id,
        before,
        after);

    assert(change.Before().Properties()[0].Value() == "3000");
    assert(change.After().Properties()[0].Value() == "3500");

    TransactionExecutor executor(store);

    assert(executor.ApplyUndo(change));
    assert(entity->Properties().All()[0].Value() == "3000");

    assert(executor.ApplyRedo(change));
    assert(entity->Properties().All()[0].Value() == "3500");

    return 0;
}
