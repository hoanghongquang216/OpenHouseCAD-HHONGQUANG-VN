#include <cassert>

#include <openhouse/model/HistoryManager.hpp>
#include <openhouse/model/TransactionExecutor.hpp>
#include <openhouse/model/EntityStateSnapshot.hpp>
#include <openhouse/model/EntitySnapshot.hpp>

using namespace openhouse::model;

int main()
{
    HistoryManager history;

    assert(!history.CanUndo());
    assert(!history.CanRedo());

    // Verification scenario:
    // Create Entity
    // Add property state
    // Capture Before snapshot
    // Modify
    // Commit HistoryEntry
    // Undo -> restore Before state
    // Redo -> restore After state

    EntitySnapshot beforeEntity(EntityId{}, EntityType{});
    EntityStateSnapshot before(beforeEntity);

    EntitySnapshot afterEntity(EntityId{}, EntityType{});
    EntityStateSnapshot after(afterEntity);

    (void)before;
    (void)after;

    return 0;
}
