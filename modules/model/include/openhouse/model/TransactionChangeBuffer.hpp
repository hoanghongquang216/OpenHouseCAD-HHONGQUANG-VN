#pragma once

#include <vector>

#include <openhouse/model/EntityChangeRecord.hpp>

namespace openhouse::model
{

class TransactionChangeBuffer
{
public:
    void Add(EntityChangeRecord record)
    {
        changes_.push_back(record);
    }

    std::size_t Count() const
    {
        return changes_.size();
    }

    void Clear()
    {
        changes_.clear();
    }

private:
    std::vector<EntityChangeRecord> changes_;
};

}
