#include "Number.h"

namespace lua
{
    std::size_t Number::GetHash() const
    {
        return 0;
    }

    bool Number::IsEqual(Value *other) const
    {
        return false;
    }
} // namespace lua
