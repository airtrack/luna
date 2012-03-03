#include "String.h"

namespace lua
{
    std::size_t String::GetHash() const
    {
        return 0;
    }

    bool String::IsEqual(Value *other) const
    {
        return false;
    }
} // namespace lua
