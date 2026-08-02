#pragma once

#include <openhouse/model/TransactionChangeBuffer.hpp>

namespace openhouse::model
{

class TransactionOperationContext
{
public:
    explicit TransactionOperationContext(TransactionChangeBuffer& buffer)
        : buffer_(buffer)
    {
    }

    TransactionChangeBuffer& Changes()
    {
        return buffer_;
    }

private:
    TransactionChangeBuffer& buffer_;
};

}
