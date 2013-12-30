#include "Bootstrap.h"
#include "State.h"

namespace luna
{
    Bootstrap::Bootstrap(State *state)
        : state_(state)
    {
    }

    void Bootstrap::Prepare()
    {
        boot_ = Instruction::AsBxCode(OpType_Call, 0, 0);

        CallInfo call;
        call.register_ = state_->stack_.top_ - 1;
        call.instruction_ = &boot_;
        call.end_ = call.instruction_ + 1;

        state_->calls_.push_back(call);
    }
} // namespace luna
