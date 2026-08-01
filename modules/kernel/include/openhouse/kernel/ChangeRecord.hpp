#pragma once

namespace openhouse::kernel
{

class ChangeRecord
{
public:
    explicit ChangeRecord(unsigned long long objectId = 0)
        : objectId_(objectId)
    {
    }

    unsigned long long ObjectId() const
    {
        return objectId_;
    }

private:
    unsigned long long objectId_;
};

}
