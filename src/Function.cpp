#include "Function.h"

namespace luna
{
    Function::Function()
        : module_(nullptr), line_(0), args_(0), is_vararg_(false)
    {
    }

    const Instruction * Function::GetOpCodes() const
    {
        return opcodes_.empty() ? nullptr : &opcodes_[0];
    }

    std::size_t Function::OpCodeSize() const
    {
        return opcodes_.size();
    }
} // namespace luna
