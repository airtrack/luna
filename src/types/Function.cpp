#include "Function.h"

namespace lua
{
    Function::Function(std::unique_ptr<UpvalueNameSet> &&upvalue_set)
        : upvalue_set_(std::move(upvalue_set))
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

    void Function::Mark()
    {
        MarkSelf();
    }

    bool Function::HasUpvalue() const
    {
        return !upvalue_set_->Empty();
    }

    const UpvalueNameSet * Function::GetUpvalueSet() const
    {
        return upvalue_set_.get();
    }
} // namespace lua
