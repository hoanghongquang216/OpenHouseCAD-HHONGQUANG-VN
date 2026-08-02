#include <cassert>

#include <openhouse/model/TransactionalModelStore.hpp>
#include <openhouse/model/TransactionHistoryStore.hpp>

int main()
{
    openhouse::model::ModelStore store;
    openhouse::model::TransactionalModelStore transactionalStore(store);

    transactionalStore.Begin();

    auto id = transactionalStore.Create(openhouse::model::EntityType{1});

    assert(id.IsValid());
    assert(store.Count() == 1);
    assert(transactionalStore.IsActive());

    transactionalStore.Commit();

    assert(!transactionalStore.IsActive());

    return 0;
}
