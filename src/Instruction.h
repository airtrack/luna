#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "types/Value.h"
#include "types/String.h"
#include <vector>

namespace lua
{
    enum OpCode
    {
        OpCode_Assign,
        OpCode_CleanStack,
        OpCode_GetLocalTable,
        OpCode_GetTable,
        OpCode_GetTableValue,
        OpCode_Push,
        OpCode_Pop,
        OpCode_GenerateClosure,
        OpCode_Ret,
        OpCode_GenerateArgTable,
        OpCode_MergeCounter,
        OpCode_ResetCounter,
        OpCode_DuplicateCounter,
        OpCode_Call,
        OpCode_AddLocalTable,
        OpCode_DelLocalTable,
        OpCode_AddGlobalTable,
        OpCode_DelGlobalTable,
        OpCode_Power,
        OpCode_Multiply,
        OpCode_Divide,
        OpCode_Mod,
        OpCode_Plus,
        OpCode_Minus,
        OpCode_Concat,
        OpCode_Less,
        OpCode_Greater,
        OpCode_LessEqual,
        OpCode_GreaterEqual,
        OpCode_NotEqual,
        OpCode_Equal,
        OpCode_Not,
        OpCode_Length,
        OpCode_Negative,
        OpCode_JmpTrue,
        OpCode_JmpFalse,
        OpCode_NewTable,
        OpCode_SetTableArrayValue,
    };

    enum InstructionParamType
    {
        InstructionParamType_Value,
        InstructionParamType_Name,
        InstructionParamType_Counter,
        InstructionParamType_CounterIndex,
        InstructionParamType_OpCodeIndex,
        InstructionParamType_ArrayIndex,
    };

    struct InstructionParam
    {
        InstructionParamType type;
        union
        {
            Value *value;
            String *name;
            int counter;
            int counter_index;
            int opcode_index;
            int array_index;
        } param;
    };

    struct Instruction
    {
        OpCode op_code;
        InstructionParam param_a;
        InstructionParam param_b;
    };

    class InstructionSet
    {
    public:
        // New one instruction and return it.
        Instruction * NewInstruction()
        {
            instructions_.resize(instructions_.size() + 1);
            return &instructions_.back();
        }

        // Get instructions array base pointer.
        Instruction * GetInstructions()
        {
            if (instructions_.empty())
                return 0;
            return &instructions_[0];
        }

        // Get total instructions count.
        std::size_t GetInstructionCount() const
        {
            return instructions_.size();
        }

    private:
        std::vector<Instruction> instructions_;
    };
} // namespace lua

#endif // INSTRUCTION_H
