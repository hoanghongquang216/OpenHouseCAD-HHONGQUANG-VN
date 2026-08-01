#pragma once

#include <cstdint>

namespace openhouse::kernel
{

class DocumentId
{
public:
    using ValueType = std::uint64_t;

    explicit DocumentId(ValueType value = 0)
        : value_(value)
    {
    }

    ValueType Value() const
    {
        return value_;
    }

    bool operator==(const DocumentId& other) const
    {
        return value_ == other.value_;
    }

    bool operator!=(const DocumentId& other) const
    {
        return !(*this == other);
    }

private:
    ValueType value_;
};

}
