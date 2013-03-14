#include "VM.h"
#include "State.h"
#include "Function.h"
#include <assert.h>

namespace luna
{
#define GET_CONST_VALUE(i)      (proto->GetConstValue(Instruction::GetParamB(i)))
#define GET_REGISTER_A(i)       (call->register_begin_ + Instruction::GetParamA(i))
#define GET_REGISTER_B(i)       (call->register_begin_ + Instruction::GetParamB(i))

    VM::VM(State *state) : state_(state)
    {
    }

    void VM::Execute()
    {
        assert(!state_->calls_.empty());

        CallInfo *call = &state_->calls_.back();
        Closure *cl = call->func_->closure_;
        Function *proto = cl->GetPrototype();
        const Instruction *instr_begin = proto->GetOpCodes();
        const Instruction *instr_end = instr_begin + proto->OpCodeSize();
        Value *a = nullptr;
        Value *b = nullptr;

        while (call->instruction_ < instr_end)
        {
            Instruction i = *call->instruction_++;

            switch (Instruction::GetOpCode(i)) {
                case OpType_LoadConst:
                    a = GET_REGISTER_A(i);
                    b = GET_CONST_VALUE(i);
                    *a = *b;
                    break;
                case OpType_Move:
                    a = GET_REGISTER_A(i);
                    b = GET_REGISTER_B(i);
                    *a = *b;
                    break;
            }
        }
    }
} // namespace luna
