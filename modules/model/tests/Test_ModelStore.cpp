#include <cassert>

#include <openhouse/model/ModelStore.hpp>

int main()
{
    openhouse::model::ModelStore store;

    auto id = store.Create(openhouse::model::EntityType{1});

    assert(id.IsValid());
    assert(store.Count() == 1);
    assert(store.Find(id) != nullptr);

    return 0;
}
