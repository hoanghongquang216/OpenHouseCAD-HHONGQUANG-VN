#pragma once

#include <openhouse/model/ModelStore.hpp>

namespace openhouse::model
{

class DocumentModel
{
public:
    ModelStore& Store()
    {
        return store_;
    }

    const ModelStore& Store() const
    {
        return store_;
    }

private:
    ModelStore store_;
};

}
