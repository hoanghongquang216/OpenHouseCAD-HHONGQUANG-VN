#include <cassert>

#include <openhouse/model/HistoryManager.hpp>
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

    EntitySnapshot beforeEntity(EntityId{}, EntityType{});
    EntityStateSnapshot before(beforeEntity);
    before.AddProperty(PropertySnapshot("Height", "3000"));

    EntitySnapshot afterEntity(EntityId{}, EntityType{});
    EntityStateSnapshot after(afterEntity);
    after.AddProperty(PropertySnapshot("Height", "3500"));

    TransactionChange change(
        ChangeOperation::Modify,
        EntityId{},
        before,
        after);

    assert(change.Before().Properties()[0].Value() == "3000");
    assert(change.After().Properties()[0].Value() == "3500");

    // Next step: execute through TransactionExecutor + ModelStore.

    return 0;
}
