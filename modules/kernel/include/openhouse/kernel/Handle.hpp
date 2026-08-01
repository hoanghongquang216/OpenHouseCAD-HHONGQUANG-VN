#pragma once

#include <cstddef>

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

    explicit operator bool() const
    {
        return IsValid();
    }

    T* Get() const
    {
        return value_;
    }

    T& operator*() const
    {
        return *value_;
    }

    T* operator->() const
    {
        return value_;
    }

    void Reset()
    {
        value_ = nullptr;
    }

private:
    T* value_ = nullptr;
};

}
