#include "NativeFunctionRegister.h"
#include "NativeFunctionStackOperator.h"
#include "State.h"

namespace lua
{
    class NativeFunctionAdaptor
    {
    public:
        NativeFunctionAdaptor(const NativeFunctionProto& func, State *state)
            : state_(state),
              func_(func)
        {
        }

        void operator () ()
        {
            NativeFunctionStackOperator so(state_);
            func_(&so);
        }

    private:
        State *state_;
        NativeFunctionProto func_;
    };

    void NativeFunctionRegister::RegisterGlobal(const std::string& name,
                                                const NativeFunctionProto& func)
    {
        NativeFunctionAdaptor adaptor(func, state_);
        NativeFunction *value = state_->GetDataPool()->GetNativeFunction(adaptor);

        String *key = state_->GetDataPool()->GetString(name);
        state_->GetGlobalTable()->Assign(key, value);
    }
} // namespace lua
