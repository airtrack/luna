#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "types/Value.h"

namespace lua
{
    enum OpCode
    {
        OpCode_Move,
    };

    enum InstructionParamType
    {
        InstructionParamType_Value,
        InstructionParamType_Number,
    };

    union InstructionParam
    {
        Value *value;
        double number;
    };

    struct Instruction
    {
        OpCode op_code;
        InstructionParamType param_a_type;
        InstructionParamType param_b_type;
        InstructionParam param_a;
        InstructionParam param_b;
    };
} // namespace lua

#endif // INSTRUCTION_H
