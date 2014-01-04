#include "VM.h"
#include "State.h"
#include "Table.h"
#include "Function.h"
#include "Exception.h"
#include <assert.h>

namespace luna
{
#define GET_CONST_VALUE(i)      (proto->GetConstValue(Instruction::GetParamBx(i)))
#define GET_REGISTER_A(i)       (call->register_ + Instruction::GetParamA(i))
#define GET_REGISTER_B(i)       (call->register_ + Instruction::GetParamB(i))
#define GET_REGISTER_C(i)       (call->register_ + Instruction::GetParamC(i))
#define GET_UPVALUE_B(i)        (cl->GetUpvalue(Instruction::GetParamB(i)))
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
        Closure *cl = call->func_ ? call->func_->closure_ : nullptr;
        Function *proto = cl ? cl->GetPrototype() : nullptr;
        Value *a = nullptr;
        Value *b = nullptr;

        while (call->instruction_ < call->end_)
        {
            Instruction i = *call->instruction_++;

            switch (Instruction::GetOpCode(i)) {
                case OpType_LoadNil:
                    a = GET_REGISTER_A(i);
                    a->SetNil();
                    SET_NEW_TOP(a);
                    break;
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
                    a = GET_REGISTER_A(i);
                    if (Call(a, i)) return ;
                    break;
                case OpType_GetGlobal:
                    a = GET_REGISTER_A(i);
                    b = GET_CONST_VALUE(i);
                    *a = state_->global_.table_->GetValue(*b);
                    SET_NEW_TOP(a);
                    break;
                default:
                    break;
            }
        }

        // For bootstrap CallInfo, we use call->register_ as new top
        Value *new_top = call->func_ ? call->func_ : call->register_;
        // Reset top value
        state_->stack_.SetNewTop(new_top);
        // Set expect results
        if (call->expect_result != EXP_VALUE_COUNT_ANY)
            state_->stack_.SetNewTop(new_top + call->expect_result);

        // Pop current CallInfo, and return to last CallInfo
        state_->calls_.pop_back();
    }

    bool VM::Call(Value *a, Instruction i)
    {
        int expect_result = Instruction::GetParamsBx(i);
        if (a->type_ == ValueT_Closure)
        {
            // We need enter next ExecuteFrame
            CallClosure(a, expect_result);
            return true;
        }
        else if (a->type_ == ValueT_CFunction)
        {
            CallCFunction(a, expect_result);
            return false;
        }
        else
        {
            auto name = GetOperandName(a);
            auto line = GetCurrentInstructionLine();
            throw RuntimeException(a, name, "call", line);
            return true;
        }
    }

    void VM::CallClosure(Value *a, int expect_result)
    {
        CallInfo callee;
        Function *callee_proto = a->closure_->GetPrototype();

        callee.func_ = a;
        callee.instruction_ = callee_proto->GetOpCodes();
        callee.end_ = callee.instruction_ + callee_proto->OpCodeSize();
        callee.expect_result = expect_result;

        Value *arg = a + 1;
        Value *top = state_->stack_.top_;
        int fixed_args = callee_proto->FixedArgCount();

        // Fixed arg start from base register
        if (callee_proto->HasVararg())
        {
            callee.register_ = top;
            int count = top - arg;
            for (int i = 0; i < count && i < fixed_args; ++i)
                *top++ = *arg++;
        }
        else
        {
            callee.register_ = arg;
        }

        state_->stack_.SetNewTop(callee.register_ + fixed_args);
        state_->calls_.push_back(callee);
    }

    void VM::CallCFunction(Value *a, int expect_result)
    {
        // Push the c function CallInfo
        CallInfo callee;
        callee.register_ = a + 1;
        callee.func_ = a;
        callee.expect_result = expect_result;
        state_->calls_.push_back(callee);

        // Call c function
        CFunctionType cfunc = a->cfunc_;
        int res_count = cfunc(state_);

        Value *src = nullptr;
        if (res_count > 0)
            src = state_->stack_.top_ - res_count;

        // Copy c function result to caller stack
        Value *dst = a;
        if (expect_result == EXP_VALUE_COUNT_ANY)
        {
            for (int i = 0; i < res_count; ++i)
                *dst++ = *src++;
        }
        else
        {
            int count = std::min(expect_result, res_count);
            for (int i = 0; i < count; ++i)
                *dst++ = *src++;
            // Set all remain expect results to nil
            count = expect_result - res_count;
            for (int i = 0; i < count; ++i, ++dst)
                dst->SetNil();
        }

        // Set registers which after dst to nil
        // and set new stack top pointer
        state_->stack_.SetNewTop(dst);

        // Pop the c function CallInfo
        state_->calls_.pop_back();
    }

    const char * VM::GetOperandName(Value *a) const
    {
        assert(!state_->calls_.empty());
        auto call = &state_->calls_.back();
        assert(call->func_ && call->func_->closure_);
        auto proto = call->func_->closure_->GetPrototype();

        auto reg = a - call->register_;
        auto instruction = call->instruction_ - 1;
        auto base = proto->GetOpCodes();
        auto pc = instruction - base;
        const char *unknown_name = "?";

        // Search last instruction which dst register is reg,
        // and get the name base on the instruction
        while (instruction > base)
        {
            --instruction;
            switch (Instruction::GetOpCode(*instruction)) {
                case OpType_GetGlobal:
                    if (reg == Instruction::GetParamA(*instruction))
                    {
                        auto index = Instruction::GetParamBx(*instruction);
                        auto key = proto->GetConstValue(index);
                        if (key->type_ == ValueT_String)
                            return key->str_->GetCStr();
                        else
                            return unknown_name;
                    }
                    break;
                case OpType_Move:
                    if (reg == Instruction::GetParamA(*instruction))
                    {
                        auto src = Instruction::GetParamB(*instruction);
                        auto name = proto->SearchLocalVar(src, pc);
                        if (name)
                            return name->GetCStr();
                        else
                            return unknown_name;
                    }
                    break;
            }
        }

        return unknown_name;
    }

    int VM::GetCurrentInstructionLine() const
    {
        assert(!state_->calls_.empty());
        auto call = &state_->calls_.back();
        assert(call->func_ && call->func_->closure_);
        auto proto = call->func_->closure_->GetPrototype();

        auto index = call->instruction_ - 1 - proto->GetOpCodes();
        return proto->GetInstructionLine(index);
    }
} // namespace luna
