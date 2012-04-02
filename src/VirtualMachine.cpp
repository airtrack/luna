#include "VirtualMachine.h"
#include "State.h"
#include "Bootstrap.h"
#include "Instruction.h"
#include "types/Table.h"
#include "types/Function.h"
#include <assert.h>

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
        nest_tables_.push_back(state->GetGlobalTable());
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
                GetTableValue();
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
            case OpCode_ReserveStack:
                ReserveStack();
                break;
            case OpCode_ExtendCounter:
                ExtendCounter();
                break;
            case OpCode_MergeCounter:
                MergeCounter();
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

    void VirtualMachine::GetTableValue()
    {
        Value *key = stack_->Top()->param.value;
        stack_->Pop(2);  // Pop key and counter
        Value *table = stack_->Top()->param.value;
        stack_->Pop();   // Pop the table

        if (table->Type() != TYPE_TABLE)
        {
            // TODO: raise error
        }

        Value *v = static_cast<Table *>(table)->GetValue(key);
        stack_->Push(v);
        stack_->Push(1, 0);
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
            const NameSet *upvalue_set = func->GetUpValueSet();
            NameSet::Iterator it = upvalue_set->Begin();
            NameSet::Iterator end = upvalue_set->End();
            while (it != end)
            {
                const Value *key = *it;
                Table *owner = GetKeyOwnerTable(key);
                if (!owner)
                {
                    // If no table has key, so we insert the upvalue as
                    // global table key, value is nil.
                    owner = nest_tables_.front();
                    owner->Assign(key, data_pool_->GetNil());
                }
                upvalue_table->Assign(key, owner->GetTableValue(key));
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

    void VirtualMachine::ReserveStack()
    {
        stack_->Push();
    }

    void VirtualMachine::ExtendCounter()
    {
        // Stack top is counter
        StackValue *fill_sv = stack_->GetStackValue(-2);
        StackValue *counter = stack_->GetStackValue(-3);

        // Fill stack value
        int fill_pos = -3 - counter->param.counter.total - 1;
        StackValue *filler = stack_->GetStackValue(fill_pos);
        filler->type = fill_sv->type;
        filler->param = fill_sv->param;

        // Extend counter
        ++counter->param.counter.total;
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

    void VirtualMachine::Call()
    {
        // Pop counter
        stack_->Pop();

        Value *callee = stack_->Top()->param.value;
        // Pop callee
        stack_->Pop();

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
            // TODO: raise error
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

    Table * VirtualMachine::GetKeyOwnerTable(const Value *key)
    {
        for (auto it = nest_tables_.rbegin(); it != nest_tables_.rend(); ++it)
            if ((*it)->HaveKey(key))
                return *it;
        return 0;
    }
} // namespace lua
