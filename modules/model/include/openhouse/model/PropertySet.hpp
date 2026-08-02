#pragma once

#include <vector>

#include <openhouse/model/Property.hpp>

namespace openhouse::model
{

class PropertySet
{
public:
    void Add(Property property)
    {
        properties_.push_back(std::move(property));
    }

    std::size_t Count() const
    {
        return properties_.size();
    }

private:
    std::vector<Property> properties_;
};

}
