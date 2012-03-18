#include "Function.h"

namespace lua
{
    Function::Function(std::unique_ptr<NameSet> &&up_value_set)
        : up_value_set_(std::move(up_value_set))
    {
    }

    std::size_t Function::GetHash() const
    {
        return std::hash<const Function *>()(this);
    }

    bool Function::IsEqual(const Value *other) const
    {
        return this == other;
    }
} // namespace lua
