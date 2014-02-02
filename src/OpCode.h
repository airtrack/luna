#ifndef OP_CODE_H
#define OP_CODE_H

namespace luna
{
    enum OpType
    {
        OpType_LoadNil = 1,             // A    A: register
        OpType_LoadBool,                // AB   A: register B: 1 true 0 false
        OpType_LoadInt,                 // A    A: register Next instruction opcode is const unsigned int
        OpType_LoadConst,               // ABx  A: register Bx: const index
        OpType_Move,                    // AB   A: dst register B: src register
        OpType_GetUpvalue,              // AB   A: register B: upvalue index
        OpType_SetUpvalue,              // AB   A: register B: upvalue index
        OpType_GetGlobal,               // ABx  A: value register Bx: const index
        OpType_SetGlobal,               // ABx  A: value register Bx: const index
        OpType_Closure,                 // ABx  A: register Bx: proto index
        OpType_Call,                    // ABC  A: register B: arg value count + 1 C: expected result count + 1
        OpType_VarArg,                  // AsBx A: register sBx: expected result count
        OpType_Ret,                     // AsBx A: return value start register sBx: return value count
        OpType_JmpFalse,                // AsBx A: register sBx: diff of instruction index
        OpType_JmpTrue,                 // AsBx A: register sBx: diff of instruction index
        OpType_JmpNil,                  // AsBx A: register sBx: diff of instruction index
        OpType_Jmp,                     // sBx  sBx: diff of instruction index
        OpType_Neg,                     // A    A: operand register and dst register
        OpType_Not,                     // A    A: operand register and dst register
        OpType_Len,                     // A    A: operand register and dst register
        OpType_Add,                     // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Sub,                     // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Mul,                     // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Div,                     // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Pow,                     // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Mod,                     // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Concat,                  // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Less,                    // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Greater,                 // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_Equal,                   // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_UnEqual,                 // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_LessEqual,               // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_GreaterEqual,            // ABC  A: dst register B: operand1 register C: operand2 register
        OpType_NewTable,                // A    A: register of table
        OpType_SetTable,                // ABC  A: register of table B: key register C: value register
        OpType_GetTable,                // ABC  A: register of table B: key register C: value register
        OpType_ForInit,                 // ABC  A: var register B: limit register    C: step register
        OpType_ForStep,                 // ABC  ABC same with OpType_ForInit, next instruction sBx: diff of instruction index
    };

    struct Instruction
    {
        unsigned int opcode_;

        Instruction() : opcode_(0) { }

        Instruction(OpType op, int a, int b, int c) : opcode_(op)
        {
            opcode_ = (opcode_ << 24) | ((a & 0xFF) << 16) | ((b & 0xFF) << 8) | (c & 0xFF);
        }

        Instruction(OpType op, int a, short b) : opcode_(op)
        {
            opcode_ = (opcode_ << 24) | ((a & 0xFF) << 16) | (static_cast<int>(b) & 0xFFFF);
        }

        Instruction(OpType op, int a, unsigned short b) : opcode_(op)
        {
            opcode_ = (opcode_ << 24) | ((a & 0xFF) << 16) | (static_cast<int>(b) & 0xFFFF);
        }

        void RefillsBx(short b)
        {
            opcode_ = (opcode_ & 0xFFFF0000) | (static_cast<int>(b) & 0xFFFF);
        }

        static int GetOpCode(Instruction i)
        {
            return (i.opcode_ >> 24) & 0xFF;
        }

        static int GetParamA(Instruction i)
        {
            return (i.opcode_ >> 16) & 0xFF;
        }

        static int GetParamB(Instruction i)
        {
            return (i.opcode_ >> 8) & 0xFF;
        }

        static int GetParamC(Instruction i)
        {
            return i.opcode_ & 0xFF;
        }

        static int GetParamsBx(Instruction i)
        {
            return static_cast<short>(i.opcode_ & 0xFFFF);
        }

        static int GetParamBx(Instruction i)
        {
            return static_cast<unsigned short>(i.opcode_ & 0xFFFF);
        }

        static Instruction ABCCode(OpType op, int a, int b, int c)
        {
            return Instruction(op, a, b, c);
        }

        static Instruction ABCode(OpType op, int a, int b)
        {
            return Instruction(op, a, b, 0);
        }

        static Instruction ACode(OpType op, int a)
        {
            return Instruction(op, a, 0, 0);
        }

        static Instruction AsBxCode(OpType op, int a, int b)
        {
            return Instruction(op, a, static_cast<short>(b));
        }

        static Instruction ABxCode(OpType op, int a, int b)
        {
            return Instruction(op, a, static_cast<unsigned short>(b));
        }
    };
} // namespace luna

#endif // OP_CODE_H
