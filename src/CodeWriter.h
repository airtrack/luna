#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "Instruction.h"

namespace lua
{
    class CodeWriter
    {
    public:
        virtual ~CodeWriter() {}
        virtual Instruction * NewInstruction() = 0;
        virtual Instruction * GetInstruction(std::size_t index) = 0;
        virtual std::size_t GetInstructionCount() const = 0;
    };

    class InstructionSetWriter : public CodeWriter
    {
    public:
        InstructionSetWriter(InstructionSet *ins_set)
            : ins_set_(ins_set)
        {
        }

        virtual Instruction * NewInstruction()
        {
            return ins_set_->NewInstruction();
        }

        virtual Instruction * GetInstruction(std::size_t index)
        {
            return ins_set_->GetInstruction(index);
        }

        virtual std::size_t GetInstructionCount() const
        {
            return ins_set_->GetInstructionCount();
        }

    private:
        InstructionSet *ins_set_;
    };
} // namespace lua

#endif // CODE_WRITER_H
