#pragma once

#include <openhouse/model/EntityChangeRecord.hpp>
#include <openhouse/model/ModelStore.hpp>

namespace openhouse::model
{

class RollbackProcessor
{
public:
    bool Process(const EntityChangeRecord& change, ModelStore& store)
    {
        switch (change.Type())
        {
        case EntityChangeType::Create:
            return store.Remove(change.Id());
        }

        return false;
    }
};

}
