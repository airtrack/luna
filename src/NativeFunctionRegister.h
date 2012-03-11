#ifndef NATIVE_FUNCTION_REGISTER_H
#define NATIVE_FUNCTION_REGISTER_H

#include <string>
#include <functional>

namespace lua
{
    class State;
    class NativeFunctionStackOperator;

    typedef std::function<void (NativeFunctionStackOperator *)> NativeFunctionProto;

    class NativeFunctionRegister
    {
    public:
        explicit NativeFunctionRegister(State *state)
            : state_(state)
        {
        }

        void RegisterGlobal(const std::string& name,
                            const NativeFunctionProto& func);

    private:
        State *state_;
    };
} // namespace lua

#endif // NATIVE_FUNCTION_REGISTER_H
