#ifndef OP_CODE_H
#define OP_CODE_H

namespace luna
{
    enum OpType
    {
        OpType_LoadConst = 1,           // AB A: register B: const index
        OpType_Move,                    // AB A: register B: register    B -> A
        OpType_Call,                    // A  A: register of closure
        OpType_SetTop,                  // A  A: index of new top
    };

    struct Instruction
    {
        unsigned int opcode_;

        Instruction(OpType op, int a, int b) : opcode_(op)
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
            return i.opcode_ & 0xFFFF;
        }

        static Instruction ABCode(OpType op, int a, int b)
        {
            return Instruction(op, a, b);
        }

        static Instruction ACode(OpType op, int a)
        {
            return Instruction(op, a, 0);
        }
    };
} // namespace luna

#endif // OP_CODE_H
