#include "String.h"

namespace lua
{
    std::size_t String::GetHash() const
    {
        return std::hash<std::string>()(value_);
    }

    bool String::IsEqual(const Value *other) const
    {
        if (this == other)
            return true;

        if (Type() != other->Type())
            return false;

        return value_ == static_cast<const String *>(other)->value_;
    }
} // namespace lua
