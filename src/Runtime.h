#ifndef RUNTIME_H
#define RUNTIME_H

#include "Value.h"
#include <vector>

namespace luna
{
    class Closure;
    class Instruction;

    // Runtime stack, registers of each function is one part of stack.
    struct Stack
    {
        static const int kBaseStackSize = 10000;

        std::vector<Value> stack_;
        Value *top_;

        Stack();
        Stack(const Stack&) = delete;
        void operator = (const Stack&) = delete;

        // Set new top pointer, and [new top, old top) will be set nil
        void SetNewTop(Value *top);
        // Set new top pointer if top > top_
        void IncToNewTop(Value *top) { if (top > top_) top_ = top; }
    };

    // Function call stack info
    struct CallInfo
    {
        // register base pointer which points to Stack
        Value *register_;

        // current closure, pointer to stack Value
        Value *func_;
        // current Instruction
        const Instruction *instruction_;

        CallInfo();
    };
} // namespace luna

#endif // RUNTIME_H
