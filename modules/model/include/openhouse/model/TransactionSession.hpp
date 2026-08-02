#pragma once

#include <openhouse/model/TransactionChangeBuffer.hpp>

namespace openhouse::model
{

class TransactionSession
{
public:
    TransactionChangeBuffer& Changes()
    {
        return buffer_;
    }

    void Commit()
    {
        buffer_.Clear();
    }

    void Rollback()
    {
        buffer_.Clear();
    }

private:
    TransactionChangeBuffer buffer_;
};

}
