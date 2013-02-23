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

    void Function::AddInstruction(Instruction i, int line)
    {
        opcodes_.push_back(i);
        opcode_lines_.push_back(line);
    }

    void Function::SetHasVararg()
    {
        is_vararg_ = true;
    }

    void Function::SetFixedArgCount(int count)
    {
        args_ = count;
    }

    void Function::SetBaseInfo(String *module, int line)
    {
        module_ = module;
        line_ = line;
    }

    Closure::Closure()
        : prototype_(nullptr)
    {
    }

    Function * Closure::GetPrototype() const
    {
        return prototype_;
    }
} // namespace luna
