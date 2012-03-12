#ifndef NATIVE_FUNCTION_STACK_OPERATOR_H
#define NATIVE_FUNCTION_STACK_OPERATOR_H

#include "Stack.h"

namespace lua
{
    class State;

    class NativeFunctionStackOperator
    {
    public:
        explicit NativeFunctionStackOperator(State *state);
        ~NativeFunctionStackOperator();

        // Get input stack size.
        std::size_t GetSize() const;
        const StackValue * GetStackValue(int index) const;

        // Check the value of index is some type.
        // If index >= 0, then index is from bottom to top,
        // else (-index) is from top to bottom.
        bool IsNil(int index) const;
        bool IsBool(int index) const;
        bool IsNumber(int index) const;
        bool IsString(int index) const;
        bool IsTable(int index) const;

        // Convert the value of index to some type.
        // If the value of index is the type which convert to, then convert it,
        // else return the default value.
        // Bool default is false.
        // Number default is 0.
        // String default is empty string.
        bool ToBool(int index) const;
        double ToNumber(int index) const;
        std::string ToString(int index) const;

        // Push some value to stack.
        void PushNil();
        void PushBool(bool b);
        void PushNumber(double number);
        void PushString(const std::string& str);
        void PushTable();

    private:
        bool IsType(int index, TypeId type) const;
        void PopInputParams();
        void PushReturnValueCounter();

        State *state_;
        Stack input_;
        int pushed_;
    };
} // namespace lua

#endif // NATIVE_FUNCTION_STACK_OPERATOR_H
