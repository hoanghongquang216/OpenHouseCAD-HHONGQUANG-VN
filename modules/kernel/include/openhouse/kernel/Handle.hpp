#pragma once

namespace openhouse::kernel
{

template<class T>
class Handle
{
public:
    Handle() = default;

    explicit Handle(T* value)
        : value_(value)
    {
    }

    bool IsValid() const
    {
        return value_ != nullptr;
    }

    T* Get() const
    {
        return value_;
    }

    T* operator->() const
    {
        return value_;
    }

private:
    T* value_ = nullptr;
};

}
