#include "VM.h"
#include "State.h"
#include "Function.h"
#include <assert.h>

namespace luna
{
#define GET_CONST_VALUE(i)      (proto->GetConstValue(Instruction::GetParamB(i)))
#define GET_REGISTER_A(i)       (call->register_ + Instruction::GetParamA(i))
#define GET_REGISTER_B(i)       (call->register_ + Instruction::GetParamB(i))

    VM::VM(State *state) : state_(state)
    {
    }

    void VM::Execute()
    {
        assert(!state_->calls_.empty());

        while (!state_->calls_.empty())
            ExecuteFrame();
    }

    void VM::ExecuteFrame()
    {
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
                case OpType_Call:
                {
                    a = GET_REGISTER_A(i);
                    if (a->type_ != ValueT_Closure)
                    {
                        // TODO: report error
                        return ;
                    }

                    CallInfo callee;
                    Function *callee_proto = a->closure_->GetPrototype();

                    callee.register_ = a + 1;
                    callee.func_ = a;
                    callee.instruction_ = callee_proto->GetOpCodes();
                    state_->calls_.push_back(callee);
                    return ;
                }
            }
        }

        // Pop current CallInfo, and return to last CallInfo
        state_->calls_.pop_back();
    }
} // namespace luna
