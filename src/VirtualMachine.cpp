#include "VirtualMachine.h"
#include "Error.h"
#include "State.h"
#include "Bootstrap.h"
#include "Instruction.h"
#include "types/Table.h"
#include "types/Function.h"
#include <assert.h>
#include <math.h>
#include <sstream>

namespace lua
{
    VirtualMachine::VirtualMachine()
        : state_(0),
          stack_(0),
          data_pool_(0),
          ins_base_(0),
          ins_count_(0),
          ins_current_(0),
          native_func_ret_(new Bootstrap)
    {
        native_func_ret_->NewInstruction()->op_code = OpCode_Ret;
    }

    void VirtualMachine::Init(State *state)
    {
        state_ = state;
        stack_ = state->GetStack();
        data_pool_ = state->GetDataPool();
    }

    void VirtualMachine::Run(Bootstrap *boot)
    {
        ins_base_ = boot->GetInstructions();
        ins_count_ = boot->GetInstructionCount();
        ins_current_ = 0;

        while (ins_current_ < ins_count_)
        {
            Instruction *ins = &ins_base_[ins_current_];
            switch (ins->op_code)
            {
            case OpCode_Assign:
                Assign();
                break;
            case OpCode_CleanStack:
                CleanStack();
                break;
            case OpCode_GetLocalTable:
                GetLocalTable();
                break;
            case OpCode_GetTable:
                GetTable(ins);
                break;
            case OpCode_GetTableValue:
                GetTableValue(ins);
                break;
            case OpCode_Push:
                DoPush(ins);
                break;
            case OpCode_GenerateClosure:
                GenerateClosure(ins);
                break;
            case OpCode_Ret:
                Return();
                break;
            case OpCode_GenerateArgTable:
                GenerateArgTable();
                break;
            case OpCode_MergeCounter:
                MergeCounter();
                break;
            case OpCode_ResetCounter:
                ResetCounter();
                break;
            case OpCode_DuplicateCounter:
                DuplicateCounter(ins);
                break;
            case OpCode_Call:
                Call();
                break;
            case OpCode_AddLocalTable:
                AddLocalTable();
                break;
            case OpCode_DelLocalTable:
                DelLocalTable();
                break;
            case OpCode_AddGlobalTable:
                AddGlobalTable();
                break;
            case OpCode_DelGlobalTable:
                DelGlobalTable();
                break;
            case OpCode_Power:
                Power();
                break;
            case OpCode_Multiply:
                Multiply();
                break;
            case OpCode_Divide:
                Divide();
                break;
            case OpCode_Mod:
                Mod();
                break;
            case OpCode_Plus:
                Plus();
                break;
            case OpCode_Minus:
                Minus();
                break;
            case OpCode_Concat:
                Concat();
                break;
            case OpCode_Less:
                Less();
                break;
            case OpCode_Greater:
                Greater();
                break;
            case OpCode_LessEqual:
                LessEqual();
                break;
            case OpCode_GreaterEqual:
                GreaterEqual();
                break;
            case OpCode_NotEqual:
                NotEqual();
                break;
            case OpCode_Equal:
                Equal();
                break;
            case OpCode_Not:
                Not();
                break;
            case OpCode_Length:
                Length();
                break;
            case OpCode_Negative:
                Negative();
                break;
            case OpCode_JmpTrue:
                JmpTrue(ins);
                break;
            case OpCode_JmpFalse:
                JmpFalse(ins);
                break;
            }
            ++ins_current_;
        }
    }

    void VirtualMachine::Assign()
    {
        assert(stack_->Top()->type == StackValueType_Value);
        Value *key = stack_->Top()->param.value;
        // Pop the key and counter
        stack_->Pop(2);

        assert(stack_->Top()->type == StackValueType_Value);
        Value *table = stack_->Top()->param.value;
        stack_->Pop();

        StackValue *counter = stack_->Top();
        assert(counter->type == StackValueType_Counter);
        Value *value = data_pool_->GetNil();

        if (counter->param.counter.current < counter->param.counter.total)
        {
            int index = counter->param.counter.current - counter->param.counter.total - 1;
            counter->param.counter.current++;
            value = stack_->GetStackValue(index)->param.value;
        }

        Table *t = static_cast<Table *>(table);
        t->Assign(key, value);
    }

    void VirtualMachine::CleanStack()
    {
        StackValue *counter = stack_->Top();
        assert(counter && counter->type == StackValueType_Counter);
        int total = counter->param.counter.total;

        // Pop the counter
        stack_->Pop();

        // Pop values bottom of the counter
        if (total > 0)
            stack_->Pop(total);
    }

    void VirtualMachine::GetLocalTable()
    {
        stack_->Push(nest_tables_.back());
        stack_->Push(1, 0);
    }

    void VirtualMachine::GetTable(Instruction *ins)
    {
        assert(ins->param_a.type == InstructionParamType_Name);
        Value *key = ins->param_a.param.name;

        std::size_t callee_tables = call_stack_.back().callee_tables;

        for (auto it = nest_tables_.rbegin(); callee_tables > 0; ++it, --callee_tables)
        {
            Table *t = *it;
            if (t->HaveKey(key))
            {
                stack_->Push(t);
                stack_->Push(1, 0);
                return ;
            }
        }

        // If can not find in local tables, we use the upvalue table.
        Value *callee = call_stack_.back().callee;
        assert(callee->Type() == TYPE_FUNCTION);
        Closure *cl = static_cast<Closure *>(callee);

        stack_->Push(cl->GetUpvalueTable());
        stack_->Push(1, 0);
    }

    void VirtualMachine::GetTableValue(Instruction *ins)
    {
        // Search to the counter
        int index = -1;
        int counter_index = ins->param_a.param.counter_index;
        for (int i = 0; i < counter_index; ++i)
        {
            --index;
            assert(stack_->GetStackValue(index)->type == StackValueType_Counter);
            index -= stack_->GetStackValue(index)->param.counter.total;
        }

        assert(stack_->GetStackValue(index - 1)->type == StackValueType_Counter);

        // -1: key, index - 1: counter, index - 2: table
        StackValue *sv = stack_->GetStackValue(index - 2);
        Value *table = sv->param.value;

        if (table->Type() != TYPE_TABLE)
        {
            throw RuntimeError("attempt to index value from " + table->Name());
        }

        Value *key = stack_->Top()->param.value;
        Value *v = static_cast<Table *>(table)->GetValue(key);
        sv->param.value = v;

        // Pop the key
        stack_->Pop();
    }

    void VirtualMachine::DoPush(Instruction *ins)
    {
        if (ins->param_a.type == InstructionParamType_Name)
            stack_->Push(ins->param_a.param.name);
        else if (ins->param_a.type == InstructionParamType_Value)
            stack_->Push(ins->param_a.param.value);
        else if (ins->param_a.type == InstructionParamType_Counter)
            stack_->Push(ins->param_a.param.counter, 0);
    }

    void VirtualMachine::GenerateClosure(Instruction *ins)
    {
        assert(ins->param_a.param.value->Type() == TYPE_FUNCTION);
        Function *func = static_cast<Function *>(ins->param_a.param.value);
        Closure *cl = data_pool_->GetClosure(func);
        stack_->Push(cl);
        stack_->Push(1, 0);

        Table *upvalue_table = cl->GetUpvalueTable();
        if (upvalue_table)
        {
            const UpvalueNameSet *upvalue_set = func->GetUpvalueSet();
            UpvalueNameSet::iterator it = upvalue_set->Begin();
            UpvalueNameSet::iterator end = upvalue_set->End();
            while (it != end)
            {
                Table *t = GetUpvalueKeyOwnerTable(it->first);
                upvalue_table->Assign(it->first, t->GetTableValue(it->first));
                ++it;
            }
        }
    }

    void VirtualMachine::Return()
    {
        CallStackInfo& call_info = call_stack_.back();
        ins_base_ = call_info.caller_base;
        ins_count_ = call_info.caller_total;
        ins_current_ = call_info.caller_offset;
        nest_tables_.resize(nest_tables_.size() - call_info.callee_tables);

        call_stack_.pop_back();
    }

    void VirtualMachine::GenerateArgTable()
    {
        Table *arg = data_pool_->GetTable();
        StackValue *sv = stack_->Top();

        int current = sv->param.counter.current;
        int total = sv->param.counter.total;
        int index = -1 - (total - current);
        int arg_index = 1;

        while (current < total)
        {
            Value *key = data_pool_->GetNumber(arg_index);
            StackValue *arg_value = stack_->GetStackValue(index);
            arg->Assign(key, arg_value->param.value);

            ++index;
            ++arg_index;
            ++current;
        }

        sv->param.counter.current = sv->param.counter.total;
        Table *local = nest_tables_.back();
        local->Assign(data_pool_->GetString("arg"), arg);
    }

    void VirtualMachine::MergeCounter()
    {
        int counter1 = stack_->GetStackValue(-1)->param.counter.total;
        int index = -1 - counter1 - 1;
        int counter2 = stack_->GetStackValue(index)->param.counter.total;

        for (int i = 0; i < counter1; ++i)
        {
            StackValue *dst = stack_->GetStackValue(index);
            StackValue *src = stack_->GetStackValue(index + 1);
            dst->type = src->type;
            dst->param = src->param;
            ++index;
        }

        stack_->Pop(2);
        stack_->Push(counter1 + counter2, 0);
    }

    void VirtualMachine::ResetCounter()
    {
        assert(stack_->Top()->type == StackValueType_Counter);
        std::size_t counter = stack_->Top()->param.counter.total;

        // counter is ok
        if (counter == 1)
            return ;

        // Pop older counter
        stack_->Pop();

        if (counter == 0)
            stack_->Push(data_pool_->GetNil());
        else
            stack_->Pop(counter - 1);

        // Push new counter
        stack_->Push(1, 0);
    }

    void VirtualMachine::DuplicateCounter(Instruction *ins)
    {
        // Search counter
        int index = -1;
        int counter_index = ins->param_a.param.counter_index;
        for (int i = 0; i < counter_index; ++i)
        {
            assert(stack_->GetStackValue(index)->type == StackValueType_Counter);
            index -= stack_->GetStackValue(index)->param.counter.total;
            --index;
        }

        assert(stack_->GetStackValue(index)->type == StackValueType_Counter);
        int total = stack_->GetStackValue(index)->param.counter.total;

        // Adjust stack index which is from bottom to top
        int stack_size = stack_->Size();
        index += stack_size - total;

        // Copy counter elements
        for (int i = 0; i < total; ++i, ++index)
        {
            StackValue *src = stack_->GetStackValue(index);
            StackValue *dst = stack_->Push();
            dst->type = src->type;
            dst->param = src->param;
        }

        // Push new counter
        stack_->Push(total, 0);
    }

    void VirtualMachine::Call()
    {
        StackValue *sv = stack_->Top();
        assert(sv->type == StackValueType_Counter);  // params counter

        int index = -1 - sv->param.counter.total;
        sv = stack_->GetStackValue(--index);
        assert(sv->type == StackValueType_Counter);  // caller counter
        assert(sv->param.counter.total == 1);        // caller counter must 1

        Value *callee = stack_->GetStackValue(--index)->param.value;
        call_stack_.push_back(CallStackInfo(ins_base_, ins_count_, ins_current_, callee));

        int type = callee->Type();
        if (type == TYPE_FUNCTION)
        {
            ins_base_ = static_cast<Closure *>(callee)->GetInstructions();
            ins_count_ = static_cast<Closure *>(callee)->GetInstructionCount();
            ins_current_ = -1;
        }
        else if (type == TYPE_NATIVE_FUNCTION)
        {
            static_cast<NativeFunction *>(callee)->Call();
            ins_base_ = native_func_ret_->GetInstructions();
            ins_count_ = native_func_ret_->GetInstructionCount();
            ins_current_ = -1;
        }
        else
        {
            throw RuntimeError("attempt to call " + callee->Name());
        }
    }

    void VirtualMachine::AddLocalTable()
    {
        nest_tables_.push_back(data_pool_->GetTable());
        ++call_stack_.back().callee_tables;
    }

    void VirtualMachine::DelLocalTable()
    {
        nest_tables_.pop_back();
        --call_stack_.back().callee_tables;
    }

    void VirtualMachine::AddGlobalTable()
    {
        nest_tables_.push_back(state_->GetGlobalTable());
        call_stack_.push_back(CallStackInfo(0, 0, 0, 0));
        call_stack_.back().callee_tables = 1;
    }

    void VirtualMachine::DelGlobalTable()
    {
        nest_tables_.pop_back();
        call_stack_.pop_back();
    }

    void VirtualMachine::Power()
    {
        double base = 0;
        double power = 0;
        CheckOperand(base, power);
        SetOperResult(pow(base, power));
    }

    void VirtualMachine::Multiply()
    {
        double left = 0;
        double right = 0;
        CheckOperand(left, right);
        SetOperResult(left * right);
    }

    void VirtualMachine::Divide()
    {
        double denominator = 0;
        double numerator = 0;
        CheckOperand(denominator, numerator);
        SetOperResult(denominator / numerator);
    }

    void VirtualMachine::Mod()
    {
        double denominator = 0;
        double numerator = 0;
        CheckOperand(denominator, numerator);
        SetOperResult(fmod(denominator, numerator));
    }

    void VirtualMachine::Plus()
    {
        double left = 0;
        double right = 0;
        CheckOperand(left, right);
        SetOperResult(left + right);
    }

    void VirtualMachine::Minus()
    {
        double left = 0;
        double right = 0;
        CheckOperand(left, right);
        SetOperResult(left - right);
    }

#define ASSERT_COUNTER(index, counter_total)                                    \
    assert(stack_->GetStackValue(index)->type == StackValueType_Counter);       \
    assert(stack_->GetStackValue(index)->param.counter.total == counter_total)

    void VirtualMachine::Concat()
    {
        ASSERT_COUNTER(-1, 1);
        const Value *right = stack_->GetStackValue(-2)->param.value;
        if (right->Type() != TYPE_STRING && right->Type() != TYPE_NUMBER)
            throw RuntimeError("attempt concat " + right->Name() + " type value");

        ASSERT_COUNTER(-3, 1);
        const Value *left = stack_->GetStackValue(-4)->param.value;
        if (left->Type() != TYPE_STRING && left->Type() != TYPE_NUMBER)
            throw RuntimeError("attempt concat " + left->Name() + " type value");

        std::ostringstream oss;
        if (left->Type() == TYPE_STRING)
            oss << static_cast<const String *>(left)->Get();
        else
            oss << static_cast<const Number *>(left)->Get();
        if (right->Type() == TYPE_STRING)
            oss << static_cast<const String *>(right)->Get();
        else
            oss << static_cast<const Number *>(right)->Get();

        SetOperResult(data_pool_->GetString(oss.str()));
    }

#define COMPLETE_COMPARE_OPERATION(oper)                                                                    \
    ASSERT_COUNTER(-1, 1);                                                                                  \
    ASSERT_COUNTER(-3, 1);                                                                                  \
    const Value *right = stack_->GetStackValue(-2)->param.value;                                            \
    const Value *left = stack_->GetStackValue(-4)->param.value;                                             \
    int right_type = right->Type();                                                                         \
    int left_type = left->Type();                                                                           \
                                                                                                            \
    if (left_type != right_type || (left_type != TYPE_STRING && left_type != TYPE_NUMBER))                  \
        throw RuntimeError("attempt to compare " + left->Name() + " with " + right->Name());                \
                                                                                                            \
    bool result = false;                                                                                    \
    if (left_type == TYPE_STRING)                                                                           \
        result = static_cast<const String *>(left)->Get() oper static_cast<const String *>(right)->Get();   \
    else                                                                                                    \
        result = static_cast<const Number *>(left)->Get() oper static_cast<const Number *>(right)->Get();   \
                                                                                                            \
    SetOperResult(data_pool_->GetBool(result))

    void VirtualMachine::Less()
    {
        COMPLETE_COMPARE_OPERATION(<);
    }

    void VirtualMachine::Greater()
    {
        COMPLETE_COMPARE_OPERATION(>);
    }

    void VirtualMachine::LessEqual()
    {
        COMPLETE_COMPARE_OPERATION(<=);
    }

    void VirtualMachine::GreaterEqual()
    {
        COMPLETE_COMPARE_OPERATION(>=);
    }

    void VirtualMachine::NotEqual()
    {
        bool result = !IsOperandsEqual();
        SetOperResult(data_pool_->GetBool(result));
    }

    void VirtualMachine::Equal()
    {
        bool result = IsOperandsEqual();
        SetOperResult(data_pool_->GetBool(result));
    }

    void VirtualMachine::Not()
    {
        ASSERT_COUNTER(-1, 1);
        StackValue *sv = stack_->GetStackValue(-2);
        bool result = !IsValueTrue(sv->param.value);
        sv->param.value = data_pool_->GetBool(result);
    }

    void VirtualMachine::Length()
    {
        ASSERT_COUNTER(-1, 1);
        StackValue *sv = stack_->GetStackValue(-2);
        const Value *value = sv->param.value;
        int type = value->Type();

        double result = 0;
        if (type == TYPE_TABLE)
            result = static_cast<const Table *>(value)->GetArraySize();
        else if (type == TYPE_STRING)
            result = static_cast<const String *>(value)->Get().size();
        else
            throw RuntimeError("attempt to get length of " + value->Name() + " type value");

        sv->param.value = data_pool_->GetNumber(result);
    }

    void VirtualMachine::Negative()
    {
        ASSERT_COUNTER(-1, 1);
        StackValue *sv = stack_->GetStackValue(-2);
        const Value *value = sv->param.value;
        if (value->Type() != TYPE_NUMBER)
            throw RuntimeError("attempt to negative " + value->Name() + " type value");

        double result = -static_cast<const Number *>(value)->Get();
        sv->param.value = data_pool_->GetNumber(result);
    }

    void VirtualMachine::JmpTrue(Instruction *ins)
    {
        ASSERT_COUNTER(-1, 1);
        const Value *value = stack_->GetStackValue(-2)->param.value;
        if (IsValueTrue(value))
        {
            // value is true, then jmp
            ins_current_ = ins->param_a.param.opcode_index;
        }
    }

    void VirtualMachine::JmpFalse(Instruction *ins)
    {
        ASSERT_COUNTER(-1, 1);
        const Value *value = stack_->GetStackValue(-2)->param.value;
        if (!IsValueTrue(value))
        {
            // value is not true, then jmp
            ins_current_ = ins->param_a.param.opcode_index;
        }
    }

    void VirtualMachine::CheckOperand(double& left, double& right)
    {
        ASSERT_COUNTER(-1, 1);
        const StackValue *sv = stack_->GetStackValue(-2);
        if (sv->param.value->Type() != TYPE_NUMBER)
            throw RuntimeError("right operand type(" + sv->param.value->Name() + ") is not number");
        right = static_cast<const Number *>(sv->param.value)->Get();

        ASSERT_COUNTER(-3, 1);
        sv = stack_->GetStackValue(-4);
        if (sv->param.value->Type() != TYPE_NUMBER)
            throw RuntimeError("left operand type(" + sv->param.value->Name() + ") is not number");
        left = static_cast<const Number *>(sv->param.value)->Get();
    }

    bool VirtualMachine::IsOperandsEqual()
    {
        ASSERT_COUNTER(-1, 1);
        ASSERT_COUNTER(-3, 1);

        const Value *right = stack_->GetStackValue(-2)->param.value;
        const Value *left = stack_->GetStackValue(-4)->param.value;

        return left->IsEqual(right);
    }

    void VirtualMachine::SetOperResult(double result)
    {
        SetOperResult(data_pool_->GetNumber(result));
    }

    void VirtualMachine::SetOperResult(Value *value)
    {
        stack_->GetStackValue(-4)->param.value = value;
        stack_->Pop(2);
    }

    bool VirtualMachine::IsValueTrue(const Value *value)
    {
        int type = value->Type();
        if (type == TYPE_NIL)
            return false;

        if (type == TYPE_BOOL && !static_cast<const Bool *>(value)->Get())
            return false;

        return true;
    }

    Table * VirtualMachine::GetUpvalueKeyOwnerTable(const Value *key)
    {
        std::size_t callee_tables = call_stack_.back().callee_tables;
        for (auto it = nest_tables_.rbegin(); callee_tables > 0 && it != nest_tables_.rend(); ++it)
        {
            Table *t = *it;
            if (t->HaveKey(key))
                return t;
        }

        Value *callee = call_stack_.back().callee;
        if (callee)
        {
            Closure *cl = static_cast<Closure *>(callee);
            Table *t = cl->GetUpvalueTable();
            // Because key can not find in local tables, so upvalue table must exist,
            // and upvalue table must have key.
            assert(t && t->HaveKey(key));
            return t;
        }

        // We insert the key to the lastest table.
        assert(callee_tables == 1);
        Table *t = nest_tables_.back();
        t->Assign(key, data_pool_->GetNil());
        return t;
    }
} // namespace lua
