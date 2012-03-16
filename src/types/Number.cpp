#include "Number.h"

namespace lua
{
    std::size_t Number::GetHash() const
    {
        return std::hash<double>()(value_);
    }

    bool Number::IsEqual(const Value *other) const
    {
        if (this == other)
            return true;

        if (Type() != other->Type())
            return false;

        return value_ == static_cast<const Number *>(other)->value_;
    }
} // namespace lua
