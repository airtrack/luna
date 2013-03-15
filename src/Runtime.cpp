#include "Runtime.h"

namespace luna
{
    Stack::Stack()
        : stack_(kBaseStackSize)
    {
    }

    CallInfo::CallInfo()
        : register_(nullptr),
          func_(nullptr),
          instruction_(nullptr)
    {
    }
} // namespace luna
