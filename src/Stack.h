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
        static const int kBaseStackSize = 1000;

        explicit Stack(int base_stack_size = kBaseStackSize);

        std::size_t Size() const;

        // If index >= 0, then return the stack value of index which from bottom to top,
        // else return the stack value of (-index) which from top to bottom.
        // If index is out of bound, then return 0.
        StackValue * GetStackValue(int index);
        const StackValue * GetStackValue(int index) const;

        // Get stack top value
        StackValue * Top();
        const StackValue * Top() const;

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

        void MarkStackValues();

    private:
        std::vector<StackValue> stack_;
    };
} // namespace lua

#endif // STACK_H
