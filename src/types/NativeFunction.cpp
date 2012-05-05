#include "NativeFunction.h"

namespace lua
{
    std::size_t NativeFunction::GetHash() const
    {
        return std::hash<const NativeFunction *>()(this);
    }

    bool NativeFunction::IsEqual(const Value *other) const
    {
        return this == other;
    }

    void NativeFunction::Mark()
    {
        MarkSelf();
    }
} // namespace lua
