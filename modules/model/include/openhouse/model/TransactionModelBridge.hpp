#pragma once

namespace openhouse::model
{

// Boundary between transaction system and model system.
// This class intentionally contains no transaction implementation yet.
// It defines the dependency direction:
// Transaction -> Bridge -> ModelStore

class TransactionModelBridge
{
public:
    TransactionModelBridge() = default;
};

}
