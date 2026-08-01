#include <cassert>

#include <openhouse/kernel/DocumentContext.hpp>

int main()
{
    openhouse::kernel::DocumentContext context;

    assert(context.CurrentState() == openhouse::kernel::DocumentContext::State::Created);

    context.SetState(openhouse::kernel::DocumentContext::State::Loaded);

    assert(context.CurrentState() == openhouse::kernel::DocumentContext::State::Loaded);

    return 0;
}
