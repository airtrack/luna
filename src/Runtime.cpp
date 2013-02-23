#include "Runtime.h"

namespace luna
{
    Stack::Stack()
        : stack_(kBaseStackSize)
    {
    }

    CallInfo::CallInfo()
        : pre_callinfo_(nullptr),
          next_callinfo_(nullptr),
          register_begin_(nullptr),
          register_end_(nullptr),
          func_(nullptr)
    {
    }
} // namespace luna
