#pragma once

namespace openhouse::kernel
{

class Transaction
{
public:
    void Commit()
    {
        committed_ = true;
    }

    void Rollback()
    {
        committed_ = false;
    }

    bool IsCommitted() const
    {
        return committed_;
    }

private:
    bool committed_{false};
};

}
