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

    // Integration flow preparation:
    // Create -> Modify -> Commit -> Undo -> Verify -> Redo -> Verify
    //
    // Real state assertions will be enabled when ModelStore restore
    // execution is connected to TransactionExecutor.

    EntitySnapshot entitySnapshot(EntityId{}, EntityType{});
    EntityStateSnapshot state(entitySnapshot);

    (void)state;

    return 0;
}
