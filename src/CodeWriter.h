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

        std::size_t StartJmpInstruction(OpCode op_code)
        {
            Instruction *ins = NewInstruction();
            ins->op_code = op_code;
            ins->param_a.type = InstructionParamType_OpCodeIndex;
            return GetInstructionCount() - 1;
        }

        void CompleteJmpInstruction(std::size_t index, std::size_t opcode_index)
        {
            Instruction *ins = GetInstruction(index);
            ins->param_a.param.opcode_index = opcode_index;
        }

        void CompleteJmpInstruction(std::size_t index)
        {
            CompleteJmpInstruction(index, GetInstructionCount() - 1);
        }

        void NewJmpInstruction(OpCode op_code, std::size_t dest_index)
        {
            CompleteJmpInstruction(StartJmpInstruction(op_code), dest_index);
        }

    private:
        InstructionSet *ins_set_;
    };
} // namespace lua

#endif // CODE_WRITER_H
