#ifndef NATIVE_FUNCTION_STACK_OPERATOR_H
#define NATIVE_FUNCTION_STACK_OPERATOR_H

namespace lua
{
    class State;

    class NativeFunctionStackOperator
    {
    public:
        explicit NativeFunctionStackOperator(State *state)
            : state_(state)
        {
        }

    private:
        State *state_;
    };
} // namespace lua

#endif // NATIVE_FUNCTION_STACK_OPERATOR_H
