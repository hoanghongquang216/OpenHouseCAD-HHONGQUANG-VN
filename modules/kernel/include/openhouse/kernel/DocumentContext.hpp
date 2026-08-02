#pragma once

#include <openhouse/kernel/DocumentId.hpp>
#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/TransactionResult.hpp>
#include <openhouse/kernel/TransactionScope.hpp>

namespace openhouse::kernel
{

class DocumentContext
{
public:
    enum class State
    {
        Created,
        Loaded,
        Modified,
        Closed
    };

    explicit DocumentContext(DocumentId id = DocumentId{})
        : id_(id)
    {
    }

    DocumentId Id() const { return id_; }

    State CurrentState() const { return state_; }

    void SetState(State state) { state_ = state; }

    bool IsModified() const { return modified_; }

    void MarkModified() { modified_ = true; }

    void MarkSaved() { modified_ = false; }

    TransactionScope BeginTransaction()
    {
        return TransactionScope{};
    }

    TransactionResult CommitTransaction(TransactionScope& transaction)
    {
        auto result = transaction.Commit();

        if (result.Success() && result.ChangeCount() > 0)
        {
            MarkModified();
        }

        return result;
    }

    void RecordChange(TransactionScope& transaction)
    {
        if (transaction.ChangeCount() > 0)
        {
            MarkModified();
        }
    }

    ObjectStore& Objects() { return objects_; }

    const ObjectStore& Objects() const { return objects_; }

    void Clear()
    {
        objects_.Clear();
        modified_ = false;
    }

private:
    DocumentId id_;
    State state_{State::Created};
    bool modified_{false};
    ObjectStore objects_;
};

}
