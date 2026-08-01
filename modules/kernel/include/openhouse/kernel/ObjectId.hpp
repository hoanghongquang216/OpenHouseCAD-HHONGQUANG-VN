#pragma once

#include <cstdint>

namespace openhouse::kernel
{

class ObjectId
{
public:
    using ValueType = std::uint64_t;

    constexpr ObjectId() = default;

    constexpr ObjectId(ValueType value)
        : value_(value)
    {
    }

    constexpr ValueType Value() const
    {
        return value_;
    }

    constexpr bool operator==(const ObjectId& other) const
    {
        return value_ == other.value_;
    }

    constexpr bool operator!=(const ObjectId& other) const
    {
        return !(*this == other);
    }

private:
    ValueType value_ = 0;
};

}

namespace std
{
template<>
struct hash<openhouse::kernel::ObjectId>
{
    std::size_t operator()(const openhouse::kernel::ObjectId& id) const noexcept
    {
        return std::hash<openhouse::kernel::ObjectId::ValueType>{}(id.Value());
    }
};
}
