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
} // namespace luna

#endif // RUNTIME_H
