#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "Instruction.h"

namespace lua
{
    class CodeWriter
    {
    public:
        explicit CodeWriter(InstructionSet *ins_set)
            : ins_set_(ins_set)
        {
        }

        Instruction * NewInstruction()
        {
            return ins_set_->NewInstruction();
        }

        Instruction * GetInstruction(std::size_t index)
        {
            return ins_set_->GetInstruction(index);
        }

        std::size_t GetInstructionCount() const
        {
            return ins_set_->GetInstructionCount();
        }

    private:
        InstructionSet *ins_set_;
    };
} // namespace lua

#endif // CODE_WRITER_H
