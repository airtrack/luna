#ifndef OP_CODE_H
#define OP_CODE_H

namespace luna
{
    enum OpType
    {
        OpType_LoadConst = 1,           // AB   A: register B: const index
        OpType_Move,                    // AB   A: register B: register    B -> A
        OpType_Call,                    // AsBx A: register of closure sBx: expected result count
        OpType_SetTop,                  // A    A: index of new top
        OpType_GetUpTable,              // ABC  A: register B: index of upvalue C: key register
    };

    struct Instruction
    {
        unsigned int opcode_;

        Instruction(OpType op, int a, int b, int c) : opcode_(op)
        {
            opcode_ = (opcode_ << 24) | (a << 16) | (b << 8) | c;
        }

        Instruction(OpType op, int a, short b) : opcode_(op)
        {
            opcode_ = (opcode_ << 24) | (a << 16) | b;
        }

        Instruction(OpType op, int a, unsigned short b) : opcode_(op)
        {
            opcode_ = (opcode_ << 24) | (a << 16) | b;
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
