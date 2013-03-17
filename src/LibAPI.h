#ifndef LIB_API_H
#define LIB_API_H

#include "Value.h"

namespace luna
{
    class State;
    class Stack;

    // This class is API for library to manipulate stack,
    // stack index value is:
    // -1 ~ -n is top to bottom,
    // 0 ~ n is bottom to top.
    class LibAPI
    {
    public:
        explicit LibAPI(State *state);

        // Get value type by index from stack
        ValueT GetValueType(int index);

        // Get value from stack by index
        double GetNumber(int index);
        const char * GetString(int index);

        // Push value to stack
        void PushNumber(double num);
        void PushString(const char *string);

    private:
        // Get value by index
        Value * GetValue(int index);
        // Push value to stack, and return the value
        Value * PushValue();

        State *state_;
        Stack *stack_;
    };
} // namespace luna

#endif // LIB_API_H
