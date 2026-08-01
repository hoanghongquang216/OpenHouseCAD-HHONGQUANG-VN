#pragma once

#include <openhouse/kernel/ObjectStore.hpp>

namespace openhouse::kernel
{

class DocumentContext
{
public:
    DocumentContext() = default;

    ObjectStore& Objects()
    {
        return objects_;
    }

    const ObjectStore& Objects() const
    {
        return objects_;
    }

    void Clear()
    {
        objects_.Clear();
    }

private:
    ObjectStore objects_;
};

}
