#pragma once

#include <openhouse/kernel/DocumentId.hpp>
#include <openhouse/kernel/ObjectStore.hpp>

namespace openhouse::kernel
{

class DocumentContext
{
public:
    explicit DocumentContext(DocumentId id = DocumentId{})
        : id_(id)
    {
    }

    DocumentId Id() const
    {
        return id_;
    }

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
    DocumentId id_;
    ObjectStore objects_;
};

}
