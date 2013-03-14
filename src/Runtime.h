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

        Stack();
        Stack(const Stack&) = delete;
        void operator = (const Stack&) = delete;
    };

    // Function call stack info
    struct CallInfo
    {
        // points to previous and next function CallInfo
        CallInfo *pre_callinfo_;
        CallInfo *next_callinfo_;

        // register range(register_begin_, register_end_) points to Stack
        Value *register_begin_;
        Value *register_end_;

        // current closure, pointer to stack Value
        Value *func_;
        // current Instruction
        const Instruction *instruction_;

        CallInfo();
    };
} // namespace luna

#endif // RUNTIME_H
