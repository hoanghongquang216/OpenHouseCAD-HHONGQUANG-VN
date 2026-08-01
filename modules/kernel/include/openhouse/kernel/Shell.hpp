#pragma once

#include <vector>

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Face;

class Shell : public TopologyEntity
{
public:
    explicit Shell(ObjectId id)
        : TopologyEntity(id)
    {
    }

    void AddFace(Face* face)
    {
        faces_.push_back(face);
    }

    const std::vector<Face*>& Faces() const
    {
        return faces_;
    }

private:
    std::vector<Face*> faces_;
};

}
