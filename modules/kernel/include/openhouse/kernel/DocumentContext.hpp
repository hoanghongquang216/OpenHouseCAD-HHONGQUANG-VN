#pragma once

#include <openhouse/kernel/DocumentId.hpp>
#include <openhouse/kernel/ObjectStore.hpp>

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

    DocumentId Id() const
    {
        return id_;
    }

    State CurrentState() const
    {
        return state_; 
    }

    void SetState(State state)
    {
        state_ = state;
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
    State state_{State::Created};
    ObjectStore objects_;
};

}
