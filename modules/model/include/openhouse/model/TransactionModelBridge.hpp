#pragma once

#include <openhouse/model/DocumentModel.hpp>

namespace openhouse::model
{

// Boundary between transaction system and model system.
// Transaction implementation remains in kernel.
// This layer only exposes model access direction.

class TransactionModelBridge
{
public:
    explicit TransactionModelBridge(DocumentModel& model)
        : model_(model)
    {
    }

    DocumentModel& Model()
    {
        return model_;
    }

    const DocumentModel& Model() const
    {
        return model_;
    }

private:
    DocumentModel& model_;
};

}
