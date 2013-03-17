#include "Runtime.h"

namespace luna
{
    Stack::Stack()
        : stack_(kBaseStackSize),
          top_(nullptr)
    {
        top_ = &stack_[0];
    }

    CallInfo::CallInfo()
        : register_(nullptr),
          func_(nullptr),
          instruction_(nullptr)
    {
    }
} // namespace luna
