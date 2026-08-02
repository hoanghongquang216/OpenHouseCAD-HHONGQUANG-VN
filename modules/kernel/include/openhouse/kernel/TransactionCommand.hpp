#pragma once

#include <openhouse/kernel/ChangeRecord.hpp>

namespace openhouse::kernel
{

class TransactionCommand
{
public:
    explicit TransactionCommand(ChangeRecord change)
        : change_(change)
    {
    }

    ChangeRecord Change() const
    {
        return change_;
    }

private:
    ChangeRecord change_;
};

}
