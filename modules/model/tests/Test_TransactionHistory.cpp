#include <cassert>

#include <openhouse/model/HistoryManager.hpp>
#include <openhouse/model/PropertySnapshot.hpp>
#include <openhouse/model/EntityStateSnapshot.hpp>
#include <openhouse/model/EntitySnapshot.hpp>

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

    assert(before.Properties().size() == 1);
    assert(after.Properties().size() == 1);
    assert(before.Properties()[0].Value() == "3000");
    assert(after.Properties()[0].Value() == "3500");

    // Next step: connect TransactionChange + Executor execution.

    return 0;
}
