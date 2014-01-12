#ifndef OP_CODE_H
#define OP_CODE_H

namespace luna
{
    enum OpType
    {
        OpType_LoadNil = 1,             // A    A: register
        OpType_LoadBool,                // AB   A: register B: 1 true 0 false
        OpType_LoadConst,               // ABx  A: register Bx: const index
        OpType_Move,                    // AB   A: dst register B: src register
        OpType_GetUpvalue,              // AB   A: register B: upvalue index
        OpType_GetGlobal,               // ABx  A: value register Bx: const index
        OpType_SetGlobal,               // ABx  A: value register Bx: const index
        OpType_Closure,                 // ABx  A: register Bx: proto index
        OpType_Call,                    // ABC  A: register B: arg value count + 1 C: expected result count + 1
        OpType_VarArg,                  // AsBx A: register sBx: expected result count
        OpType_Ret,                     // AsBx A: return value start register sBx: return value count
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
