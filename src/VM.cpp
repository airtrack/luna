#include "VM.h"
#include "State.h"
#include "Function.h"
#include <assert.h>

namespace luna
{
#define GET_CONST_VALUE(i)      (proto->GetConstValue(Instruction::GetParamB(i)))
#define GET_REGISTER_A(i)       (call->register_ + Instruction::GetParamA(i))
#define GET_REGISTER_B(i)       (call->register_ + Instruction::GetParamB(i))
#define SET_NEW_TOP(a)          (state_->stack_.IncToNewTop(a + 1))

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
                    SET_NEW_TOP(a);
                    break;
                case OpType_Move:
                    a = GET_REGISTER_A(i);
                    b = GET_REGISTER_B(i);
                    *a = *b;
                    SET_NEW_TOP(a);
                    break;
                case OpType_Call:
                {
                    a = GET_REGISTER_A(i);
                    if (a->type_ == ValueT_Closure)
                    {
                        CallClosure(a);
                        // Return, then we enter next ExecuteFrame
                        return ;
                    }
                    else if (a->type_ == ValueT_CFunction)
                    {
                        CallCFunction(a);
                    }
                    else
                    {
                        // TODO: report error
                        return ;
                    }
                    break;
                }
                case OpType_SetTop:
                    a = GET_REGISTER_A(i);
                    state_->stack_.SetNewTop(a);
                    break;
            }
        }

        // Reset top value
        state_->stack_.SetNewTop(call->func_);

        // Pop current CallInfo, and return to last CallInfo
        state_->calls_.pop_back();
    }

    void VM::CallClosure(Value *a)
    {
        CallInfo callee;
        Function *callee_proto = a->closure_->GetPrototype();

        callee.register_ = a + 1;
        callee.func_ = a;
        callee.instruction_ = callee_proto->GetOpCodes();
        state_->calls_.push_back(callee);
    }

    void VM::CallCFunction(Value *a)
    {
        // Push the c function CallInfo
        CallInfo callee;
        callee.register_ = a + 1;
        callee.func_ = a;
        state_->calls_.push_back(callee);

        // Call c function
        CFunctionType cfunc = a->cfunc_;
        int res_count = cfunc(state_);

        Value *src = nullptr;
        if (res_count > 0)
            src = state_->stack_.top_ - res_count;

        // Copy c function result to caller stack
        Value *dst = a;
        for (int i = 0; i < res_count; ++i)
            *dst++ = *src++;

        state_->stack_.SetNewTop(dst);

        // Pop the c function CallInfo
        state_->calls_.pop_back();
    }
} // namespace luna
