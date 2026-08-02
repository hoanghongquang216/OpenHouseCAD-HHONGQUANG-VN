#pragma once

#include <string>
#include <utility>

namespace openhouse::model
{

class Property
{
public:
    Property() = default;

    Property(std::string name, std::string value)
        : name_(std::move(name)), value_(std::move(value))
    {
    }

    const std::string& Name() const
    {
        return name_;
    }

    const std::string& Value() const
    {
        return value_;
    }

    void SetValue(std::string value)
    {
        value_ = std::move(value);
    }

private:
    std::string name_;
    std::string value_;
};

}
