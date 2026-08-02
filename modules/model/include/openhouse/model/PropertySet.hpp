#pragma once

#include <vector>
#include <utility>

#include <openhouse/model/Property.hpp>
#include <openhouse/model/PropertySnapshot.hpp>

namespace openhouse::model
{

class PropertySet
{
public:
    void Add(Property property)
    {
        properties_.push_back(std::move(property));
    }

    void Clear()
    {
        properties_.clear();
    }

    void Restore(std::vector<Property> properties)
    {
        properties_ = std::move(properties);
    }

    void RestoreSnapshots(const std::vector<PropertySnapshot>& snapshots)
    {
        properties_.clear();

        for (const auto& snapshot : snapshots)
        {
            properties_.emplace_back(snapshot.Name(), snapshot.Value());
        }
    }

    const std::vector<Property>& All() const
    {
        return properties_;
    }

    std::size_t Count() const
    {
        return properties_.size();
    }

private:
    std::vector<Property> properties_;
};

}
