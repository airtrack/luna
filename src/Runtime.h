#ifndef RUNTIME_H
#define RUNTIME_H

#include "Value.h"
#include <vector>

namespace luna
{
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
        // pointer previous and next function call info
        CallInfo *pre_callinfo_;
        CallInfo *next_callinfo_;

        // register range(register_begin_, register_end_) pointer to Stack
        Value *register_begin_;
        Value *register_end_;

        CallInfo();
    };
} // namespace luna

#endif // RUNTIME_H
