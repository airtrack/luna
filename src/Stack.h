#ifndef STACK_H
#define STACK_H

#include "types/Value.h"
#include <vector>

namespace lua
{
    enum StackValueType
    {
        StackValueType_Value,
        StackValueType_Counter,
    };

    struct StackValue
    {
        StackValueType type;

        union
        {
            Value *value;
            struct
            {
                int total;
                int current;
            } counter;
        } param;
    };

    class Stack
    {
    public:
        Stack();

        // If index >= 0, then return the stack value of index from bottom to top,
        // else return the stack value of (-index) from top to bottom.
        // If index is out of bound, then return 0.
        StackValue * GetStackValue(int index);

        // Get stack top value
        StackValue * Top();

        // Pop count values from stack top
        void Pop(int count = 1);

        // Clear stack
        void Clear();

        // Push new value to stack and return the new value pointer
        StackValue * Push();

        // Push a value to stack
        void Push(Value *value);

        // Push counter to stack
        void Push(int total, int current);

    private:
        static const int kBaseStackSize = 1000;

        std::vector<StackValue> stack_;
    };
} // namespace lua

#endif // STACK_H
