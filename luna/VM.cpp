#include "VM.h"
#include "State.h"
#include "Table.h"
#include "UserData.h"
#include "Function.h"
#include "Exception.h"
#include <assert.h>
#include <math.h>

namespace
{
    std::string NumberToStr(luna::Value *num)
    {
        assert(num->type_ == luna::ValueT_Number);
        char temp[64];
        if (floor(num->num_) == num->num_)
            snprintf(temp, sizeof(temp), "%lld", static_cast<long long>(num->num_));
        else
            snprintf(temp, sizeof(temp), "%g", num->num_);
        return temp;
    }
} // namespace

namespace luna
{
#define GET_CONST_VALUE(i)      (proto->GetConstValue(Instruction::GetParamBx(i)))
#define GET_REGISTER_A(i)       (call->register_ + Instruction::GetParamA(i))
#define GET_REGISTER_B(i)       (call->register_ + Instruction::GetParamB(i))
#define GET_REGISTER_C(i)       (call->register_ + Instruction::GetParamC(i))
#define GET_UPVALUE_B(i)        (cl->GetUpvalue(Instruction::GetParamB(i)))
#define GET_REAL_VALUE(a)       (a->type_ == ValueT_Upvalue ? a->upvalue_->GetValue() : a)

#define GET_REGISTER_ABC(i)                                 \
    a = GET_REGISTER_A(i);                                  \
    b = GET_REGISTER_B(i);                                  \
    c = GET_REGISTER_C(i);

#define GET_CALLINFO_AND_PROTO()                            \
    assert(!state_->calls_.empty());                        \
    auto call = &state_->calls_.back();                     \
    assert(call->func_ && call->func_->closure_);           \
    auto proto = call->func_->closure_->GetPrototype()

    VM::VM(State *state) : state_(state)
    {
    }

    void VM::Execute()
    {
        assert(!state_->calls_.empty());

        while (!state_->calls_.empty())
        {
            // If current stack frame is a frame of a c function,
            // do not continue execute instructions, just return
            if (state_->calls_.back().func_->type_ == ValueT_CFunction)
                return ;
            ExecuteFrame();
        }
    }

    void VM::ExecuteFrame()
    {
        CallInfo *call = &state_->calls_.back();
        Closure *cl = call->func_->closure_;
        Function *proto = cl->GetPrototype();
        Value *a = nullptr;
        Value *b = nullptr;
        Value *c = nullptr;

        while (call->instruction_ < call->end_)
        {
            state_->CheckRunGC();
            Instruction i = *call->instruction_++;

            switch (Instruction::GetOpCode(i)) {
                case OpType_LoadNil:
                    a = GET_REGISTER_A(i);
                    GET_REAL_VALUE(a)->SetNil();
                    break;
                case OpType_FillNil:
                    a = GET_REGISTER_A(i);
                    b = GET_REGISTER_B(i);
                    while (a < b)
                    {
                        a->SetNil();
                        ++a;
                    }
                    break;
                case OpType_LoadBool:
                    a = GET_REGISTER_A(i);
                    GET_REAL_VALUE(a)->SetBool(Instruction::GetParamB(i) ? true : false);
                    break;
                case OpType_LoadInt:
                    a = GET_REGISTER_A(i);
                    assert(call->instruction_ < call->end_);
                    a->num_ = (*call->instruction_++).opcode_;
                    a->type_ = ValueT_Number;
                    break;
                case OpType_LoadConst:
                    a = GET_REGISTER_A(i);
                    b = GET_CONST_VALUE(i);
                    *GET_REAL_VALUE(a) = *b;
                    break;
                case OpType_Move:
                    a = GET_REGISTER_A(i);
                    b = GET_REGISTER_B(i);
                    *GET_REAL_VALUE(a) = *GET_REAL_VALUE(b);
                    break;
                case OpType_Call:
                    a = GET_REGISTER_A(i);
                    if (Call(a, i)) return ;
                    break;
                case OpType_GetUpvalue:
                    a = GET_REGISTER_A(i);
                    b = GET_UPVALUE_B(i)->GetValue();
                    *GET_REAL_VALUE(a) = *b;
                    break;
                case OpType_SetUpvalue:
                    a = GET_REGISTER_A(i);
                    b = GET_UPVALUE_B(i)->GetValue();
                    *b = *a;
                    break;
                case OpType_GetGlobal:
                    a = GET_REGISTER_A(i);
                    b = GET_CONST_VALUE(i);
                    *GET_REAL_VALUE(a) = state_->global_.table_->GetValue(*b);
                    break;
                case OpType_SetGlobal:
                    a = GET_REGISTER_A(i);
                    b = GET_CONST_VALUE(i);
                    state_->global_.table_->SetValue(*b, *a);
                    break;
                case OpType_Closure:
                    a = GET_REGISTER_A(i);
                    GenerateClosure(a, i);
                    break;
                case OpType_VarArg:
                    a = GET_REGISTER_A(i);
                    CopyVarArg(a, i);
                    break;
                case OpType_Ret:
                    a = GET_REGISTER_A(i);
                    return Return(a, i);
                case OpType_JmpFalse:
                    a = GET_REGISTER_A(i);
                    if (GET_REAL_VALUE(a)->IsFalse())
                        call->instruction_ += -1 + Instruction::GetParamsBx(i);
                    break;
                case OpType_JmpTrue:
                    a = GET_REGISTER_A(i);
                    if (!GET_REAL_VALUE(a)->IsFalse())
                        call->instruction_ += -1 + Instruction::GetParamsBx(i);
                    break;
                case OpType_JmpNil:
                    a = GET_REGISTER_A(i);
                    if (a->type_ == ValueT_Nil)
                        call->instruction_ += -1 + Instruction::GetParamsBx(i);
                    break;
                case OpType_Jmp:
                    call->instruction_ += -1 + Instruction::GetParamsBx(i);
                    break;
                case OpType_Neg:
                    a = GET_REGISTER_A(i);
                    CheckType(a, ValueT_Number, "neg");
                    a->num_ = -a->num_;
                    break;
                case OpType_Not:
                    a = GET_REGISTER_A(i);
                    a->SetBool(a->IsFalse() ? true : false);
                    break;
                case OpType_Len:
                    a = GET_REGISTER_A(i);
                    if (a->type_ == ValueT_Table)
                        a->num_ = a->table_->ArraySize();
                    else if (a->type_ == ValueT_String)
                        a->num_ = a->str_->GetLength();
                    else
                        ReportTypeError(a, "length of");
                    a->type_ = ValueT_Number;
                    break;
                case OpType_Add:
                    GET_REGISTER_ABC(i);
                    CheckArithType(b, c, "add");
                    a->num_ = b->num_ + c->num_;
                    a->type_ = ValueT_Number;
                    break;
                case OpType_Sub:
                    GET_REGISTER_ABC(i);
                    CheckArithType(b, c, "sub");
                    a->num_ = b->num_ - c->num_;
                    a->type_ = ValueT_Number;
                    break;
                case OpType_Mul:
                    GET_REGISTER_ABC(i);
                    CheckArithType(b, c, "multiply");
                    a->num_ = b->num_ * c->num_;
                    a->type_ = ValueT_Number;
                    break;
                case OpType_Div:
                    GET_REGISTER_ABC(i);
                    CheckArithType(b, c, "div");
                    a->num_ = b->num_ / c->num_;
                    a->type_ = ValueT_Number;
                    break;
                case OpType_Pow:
                    GET_REGISTER_ABC(i);
                    CheckArithType(b, c, "power");
                    a->num_ = pow(b->num_, c->num_);
                    a->type_ = ValueT_Number;
                    break;
                case OpType_Mod:
                    GET_REGISTER_ABC(i);
                    CheckArithType(b, c, "mod");
                    a->num_ = fmod(b->num_, c->num_);
                    a->type_ = ValueT_Number;
                    break;
                case OpType_Concat:
                    GET_REGISTER_ABC(i);
                    Concat(a, b, c);
                    break;
                case OpType_Less:
                    GET_REGISTER_ABC(i);
                    CheckInequalityType(b, c, "compare(<)");
                    if (b->type_ == ValueT_Number)
                        a->SetBool(b->num_ < c->num_);
                    else
                        a->SetBool(*b->str_ < *c->str_);
                    break;
                case OpType_Greater:
                    GET_REGISTER_ABC(i);
                    CheckInequalityType(b, c, "compare(>)");
                    if (b->type_ == ValueT_Number)
                        a->SetBool(b->num_ > c->num_);
                    else
                        a->SetBool(*b->str_ > *c->str_);
                    break;
                case OpType_Equal:
                    GET_REGISTER_ABC(i);
                    a->SetBool(*b == *c);
                    break;
                case OpType_UnEqual:
                    GET_REGISTER_ABC(i);
                    a->SetBool(*b != *c);
                    break;
                case OpType_LessEqual:
                    GET_REGISTER_ABC(i);
                    CheckInequalityType(b, c, "compare(<=)");
                    if (b->type_ == ValueT_Number)
                        a->SetBool(b->num_ <= c->num_);
                    else
                        a->SetBool(*b->str_ <= *c->str_);
                    break;
                case OpType_GreaterEqual:
                    GET_REGISTER_ABC(i);
                    CheckInequalityType(b, c, "compare(>=)");
                    if (b->type_ == ValueT_Number)
                        a->SetBool(b->num_ >= c->num_);
                    else
                        a->SetBool(*b->str_ >= *c->str_);
                    break;
                case OpType_NewTable:
                    a = GET_REGISTER_A(i);
                    a->table_ = state_->NewTable();
                    a->type_ = ValueT_Table;
                    break;
                case OpType_SetTable:
                    GET_REGISTER_ABC(i);
                    CheckTableType(a, b, "set", "to");
                    if (a->type_ == ValueT_Table)
                        a->table_->SetValue(*b, *c);
                    else if (a->type_ == ValueT_UserData)
                        a->user_data_->GetMetatable()->SetValue(*b, *c);
                    else
                        assert(0);
                    break;
                case OpType_GetTable:
                    GET_REGISTER_ABC(i);
                    CheckTableType(a, b, "get", "from");
                    if (a->type_ == ValueT_Table)
                        *c = a->table_->GetValue(*b);
                    else if (a->type_ == ValueT_UserData)
                        *c = a->user_data_->GetMetatable()->GetValue(*b);
                    else
                        assert(0);
                    break;
                case OpType_ForInit:
                    GET_REGISTER_ABC(i);
                    ForInit(a, b, c);
                    break;
                case OpType_ForStep:
                    GET_REGISTER_ABC(i);
                    i = *call->instruction_++;
                    if ((c->num_ > 0.0 && a->num_ > b->num_) ||
                        (c->num_ <= 0.0 && a->num_ < b->num_))
                        call->instruction_ += -1 + Instruction::GetParamsBx(i);
                    break;
                default:
                    break;
            }
        }

        Value *new_top = call->func_;
        // Reset top value
        state_->stack_.SetNewTop(new_top);
        // Set expect results
        if (call->expect_result_ != EXP_VALUE_COUNT_ANY)
            state_->stack_.SetNewTop(new_top + call->expect_result_);

        // Pop current CallInfo, and return to last CallInfo
        state_->calls_.pop_back();
    }

    bool VM::Call(Value *a, Instruction i)
    {
        if (a->type_ != ValueT_Closure &&
            a->type_ != ValueT_CFunction)
        {
            ReportTypeError(a, "call");
            return true;
        }

        try
        {
            int arg_count = Instruction::GetParamB(i) - 1;
            int expect_result = Instruction::GetParamC(i) - 1;
            return state_->CallFunction(a, arg_count, expect_result);
        } catch (const CallCFuncException &e)
        {
            // Calculate line number of the call
            auto pos = GetCurrentInstructionPos();
            throw RuntimeException(pos.first, pos.second, e.What().c_str());
        }
    }

    void VM::GenerateClosure(Value *a, Instruction i)
    {
        GET_CALLINFO_AND_PROTO();
        auto a_proto = proto->GetChildFunction(Instruction::GetParamBx(i));
        a->type_ = ValueT_Closure;
        a->closure_ = state_->NewClosure();
        a->closure_->SetPrototype(a_proto);

        // Prepare all upvalues
        auto new_closure = a->closure_;
        auto closure = call->func_->closure_;
        auto count = a_proto->GetUpvalueCount();
        for (std::size_t i = 0; i < count; ++i)
        {
            auto upvalue_info = a_proto->GetUpvalue(i);
            if (upvalue_info->parent_local_)
            {
                // Transform local variable to upvalue
                auto reg = call->register_ + upvalue_info->register_index_;
                if (reg->type_ != ValueT_Upvalue)
                {
                    auto upvalue = state_->NewUpvalue();
                    upvalue->SetValue(*reg);
                    reg->type_ = ValueT_Upvalue;
                    reg->upvalue_ = upvalue;
                    new_closure->AddUpvalue(upvalue);
                }
                else
                {
                    new_closure->AddUpvalue(reg->upvalue_);
                }
            }
            else
            {
                // Get upvalue from parent upvalue list
                auto upvalue = closure->GetUpvalue(upvalue_info->register_index_);
                new_closure->AddUpvalue(upvalue);
            }
        }
    }

    void VM::CopyVarArg(Value *a, Instruction i)
    {
        GET_CALLINFO_AND_PROTO();
        auto arg = call->func_ + 1;
        int total_args = call->register_ - arg;
        int vararg_count = total_args - proto->FixedArgCount();

        arg += proto->FixedArgCount();
        int expect_count = Instruction::GetParamsBx(i);
        if (expect_count == EXP_VALUE_COUNT_ANY)
        {
            for (int i = 0; i < vararg_count; ++i)
                *a++ = *arg++;
            state_->stack_.SetNewTop(a);
        }
        else
        {
            int i = 0;
            for (; i < vararg_count && i < expect_count; ++i)
                *a++ = *arg++;
            for (; i < expect_count; ++i, ++a)
                a->SetNil();
        }
    }

    void VM::Return(Value *a, Instruction i)
    {
        // Set stack top when return value count is fixed
        int ret_value_count = Instruction::GetParamsBx(i);
        if (ret_value_count != EXP_VALUE_COUNT_ANY)
            state_->stack_.top_ = a + ret_value_count;

        assert(!state_->calls_.empty());
        auto call = &state_->calls_.back();

        auto src = a;
        auto dst = call->func_;

        int expect_result = call->expect_result_;
        int result_count = state_->stack_.top_ - src;
        if (expect_result == EXP_VALUE_COUNT_ANY)
        {
            for (int i = 0; i < result_count; ++i)
                *dst++ = *src++;
        }
        else
        {
            int i = 0;
            int count = std::min(expect_result, result_count);
            for (; i < count; ++i)
                *dst++ = *src++;
            // No enough results for expect results, set remain as nil
            for (; i < expect_result; ++i, ++dst)
                dst->SetNil();
        }

        // Set new top and pop current CallInfo
        state_->stack_.SetNewTop(dst);
        state_->calls_.pop_back();
    }

    void VM::Concat(Value *dst, Value *op1, Value *op2)
    {
        if (op1->type_ == ValueT_String && op2->type_ == ValueT_String)
        {
            dst->str_ = state_->GetString(op1->str_->GetStdString() +
                                          op2->str_->GetCStr());
        }
        else if (op1->type_ == ValueT_String && op2->type_ == ValueT_Number)
        {
            dst->str_ = state_->GetString(op1->str_->GetCStr() +
                                          NumberToStr(op2));
        }
        else if (op1->type_ == ValueT_Number && op2->type_ == ValueT_String)
        {
            dst->str_ = state_->GetString(NumberToStr(op1) +
                                          op2->str_->GetCStr());
        }
        else
        {
            auto pos = GetCurrentInstructionPos();
            throw RuntimeException(pos.first, pos.second, op1, op2, "concat");
        }

        dst->type_ = ValueT_String;
    }

    void VM::ForInit(Value *var, Value *limit, Value *step)
    {
        if (var->type_ != ValueT_Number)
        {
            auto pos = GetCurrentInstructionPos();
            throw RuntimeException(pos.first, pos.second,
                                   var, "'for' init", "number");
        }

        if (limit->type_ != ValueT_Number)
        {
            auto pos = GetCurrentInstructionPos();
            throw RuntimeException(pos.first, pos.second,
                                   limit, "'for' limit", "number");
        }

        if (step->type_ != ValueT_Number)
        {
            auto pos = GetCurrentInstructionPos();
            throw RuntimeException(pos.first, pos.second,
                                   step, "'for' step", "number");
        }
    }

    std::pair<const char *, const char *> VM::GetOperandNameAndScope(const Value *a) const
    {
        GET_CALLINFO_AND_PROTO();

        auto reg = a - call->register_;
        auto instruction = call->instruction_ - 1;
        auto base = proto->GetOpCodes();
        auto pc = instruction - base;
        const char *unknown_name = "?";
        const char *scope_global = "global";
        const char *scope_local = "local";
        const char *scope_upvalue = "upvalue";
        const char *scope_table = "table member";
        const char *scope_null = "";

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
                            return { key->str_->GetCStr(), scope_global };
                        else
                            return { unknown_name, scope_null };
                    }
                    break;
                case OpType_Move:
                    if (reg == Instruction::GetParamA(*instruction))
                    {
                        auto src = Instruction::GetParamB(*instruction);
                        auto name = proto->SearchLocalVar(src, pc);
                        if (name)
                            return { name->GetCStr(), scope_local };
                        else
                            return { unknown_name, scope_null };
                    }
                    break;
                case OpType_GetUpvalue:
                    if (reg == Instruction::GetParamA(*instruction))
                    {
                        auto index = Instruction::GetParamB(*instruction);
                        auto upvalue_info = proto->GetUpvalue(index);
                        return { upvalue_info->name_->GetCStr(), scope_upvalue };
                    }
                    break;
                case OpType_GetTable:
                    if (reg == Instruction::GetParamC(*instruction))
                    {
                        auto key = Instruction::GetParamB(*instruction);
                        auto key_reg = call->register_ + key;
                        if (key_reg->type_ == ValueT_String)
                            return { key_reg->str_->GetCStr(), scope_table };
                        else
                            return { unknown_name, scope_table };
                    }
                    break;
            }
        }

        return { unknown_name, scope_null };
    }

    std::pair<const char *, int> VM::GetCurrentInstructionPos() const
    {
        GET_CALLINFO_AND_PROTO();
        auto index = call->instruction_ - 1 - proto->GetOpCodes();
        return { proto->GetModule()->GetCStr(),
                 proto->GetInstructionLine(index) };
    }

    void VM::CheckType(const Value *v, ValueT type, const char *op) const
    {
        if (v->type_ != type)
            ReportTypeError(v, op);
    }

    void VM::CheckArithType(const Value *v1, const Value *v2, const char *op) const
    {
        if (v1->type_ != ValueT_Number || v2->type_ != ValueT_Number)
        {
            auto pos = GetCurrentInstructionPos();
            throw RuntimeException(pos.first, pos.second, v1, v2, op);
        }
    }

    void VM::CheckInequalityType(const Value *v1, const Value *v2,
                                 const char *op) const
    {
        if (v1->type_ != v2->type_ ||
            (v1->type_ != ValueT_Number && v1->type_ != ValueT_String))
        {
            auto pos = GetCurrentInstructionPos();
            throw RuntimeException(pos.first, pos.second, v1, v2, op);
        }
    }

    void VM::CheckTableType(const Value *t, const Value *k,
                            const char *op, const char *desc) const
    {
        if (t->type_ == ValueT_Table ||
            (t->type_ == ValueT_UserData && t->user_data_->GetMetatable()))
            return ;

        auto ns = GetOperandNameAndScope(t);
        auto pos = GetCurrentInstructionPos();
        auto key_name = k->type_ == ValueT_String ? k->str_->GetCStr() : "?";
        auto op_desc = std::string(op) + " table key '" + key_name + "' " + desc;

        throw RuntimeException(pos.first, pos.second, t,
                ns.first, ns.second, op_desc.c_str());
    }

    void VM::ReportTypeError(const Value *v, const char *op) const
    {
        auto ns = GetOperandNameAndScope(v);
        auto pos = GetCurrentInstructionPos();
        throw RuntimeException(pos.first, pos.second, v, ns.first, ns.second, op);
    }
} // namespace luna
