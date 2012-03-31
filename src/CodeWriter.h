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

    private:
        InstructionSet *ins_set_;
    };
} // namespace lua

#endif // CODE_WRITER_H