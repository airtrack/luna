#include "Closure.h"
#include "Function.h"

namespace lua
{
    Closure::Closure(Function *func, Table *upvalue)
        : func_(func),
          upvalue_table_(upvalue)
    {
    }

    std::size_t Closure::GetHash() const
    {
        return func_->GetHash();
    }

    bool Closure::IsEqual(const Value *other) const
    {
        return this == other;
    }

    Instruction * Closure::GetInstructions()
    {
        return func_->GetInstructions();
    }

    std::size_t Closure::GetInstructionCount() const
    {
        return func_->GetInstructionCount();
    }
} // namespace lua
