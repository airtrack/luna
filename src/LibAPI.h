#ifndef LIB_API_H
#define LIB_API_H

#include "Value.h"

namespace luna
{
    class State;
    class Stack;
    class Table;
    class Closure;

    // This class is API for library to manipulate stack,
    // stack index value is:
    // -1 ~ -n is top to bottom,
    // 0 ~ n is bottom to top.
    class StackAPI
    {
    public:
        explicit StackAPI(State *state);

        // Get count of value in this function stack
        int GetStackSize() const;

        // Get value type by index from stack
        ValueT GetValueType(int index);

        // Get value from stack by index
        double GetNumber(int index);
        const char * GetString(int index);
        bool GetBool(int index);
        Closure * GetClosure(int index);
        Table * GetTable(int index);
        CFunctionType GetCFunction(int index);

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

    // This class register C function to luna
    class Library
    {
    public:
        explicit Library(State *state);

        // Register global function 'func' as 'name'
        void RegisterFunc(const char *name, CFunctionType func);

    private:
        State *state_;
        Table *global_;
    };
} // namespace luna

#endif // LIB_API_H
