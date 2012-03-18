#include "Function.h"

namespace lua
{
    std::size_t Function::GetHash() const
    {
        return std::hash<const Function *>()(this);
    }

    bool Function::IsEqual(const Value *other) const
    {
        return this == other;
    }
} // namespace lua
