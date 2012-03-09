#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "types/Value.h"
#include "types/String.h"

namespace lua
{
    enum OpCode
    {
        OpCode_Assign,
        OpCode_CleanStack,
        OpCode_GetTable,
        OpCode_Push,
    };

    enum InstructionParamType
    {
        InstructionParamType_Value,
        InstructionParamType_Name,
        InstructionParamType_Number,
        InstructionParamType_Bool,
        InstructionParamType_Nil,
        InstructionParamType_Counter,
    };

    struct InstructionParam
    {
        InstructionParamType type;
        union
        {
            Value *value;
            String *name;
            double number;
            bool b;
            int counter;
        } param;
    };

    struct Instruction
    {
        OpCode op_code;
        InstructionParam param_a;
        InstructionParam param_b;
    };
} // namespace lua

#endif // INSTRUCTION_H
