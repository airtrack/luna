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
          data_pool_(0)
    {
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
        Instruction *instructions = boot->GetInstructions();
        std::size_t ins_count = boot->GetInstructionCount();
        std::size_t current = 0;

        while (current < ins_count)
        {
            Instruction *ins = &instructions[current];
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
                assert(ins->param_a.type == InstructionParamType_Name);
                GetTable(ins->param_a.param.name);
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
            }
            ++current;
        }
    }

    void VirtualMachine::Assign()
    {
        StackValue *key = stack_->Top();
        assert(key->type == StackValueType_Value);
        stack_->Pop();

        StackValue *table = stack_->Top();
        assert(table->type == StackValueType_Value &&
            table->param.value->Type() == TYPE_TABLE);
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

        Table *t = static_cast<Table *>(table->param.value);
        t->Assign(key->param.value, value);
    }

    void VirtualMachine::CleanStack()
    {
        StackValue *counter = stack_->Top();
        assert(counter && counter->type == StackValueType_Counter);

        // Pop the counter
        stack_->Pop();

        // Pop values bottom of the counter
        stack_->Pop(counter->param.counter.total);
    }

    void VirtualMachine::GetLocalTable()
    {
        stack_->Push(nest_tables_.back());
        stack_->Push(1, 0);
    }

    void VirtualMachine::GetTable(String *name)
    {
        for (NestTables::reverse_iterator it = nest_tables_.rbegin();
            it != nest_tables_.rend(); ++it)
        {
            Table *t = *it;
            if (t->HaveKey(name))
            {
                stack_->Push(t);
                stack_->Push(1, 0);
                return ;
            }
        }

        // If can not find key name from all tables, then we use global table.
        stack_->Push(nest_tables_.front());
        stack_->Push(1, 0);
    }

    void VirtualMachine::GetTableValue()
    {
        StackValue *key = stack_->Top();
        stack_->Pop(2);  // Pop key and counter
        StackValue *table = stack_->Top();
        stack_->Pop();   // Pop the table

        if (table->param.value->Type() != TYPE_TABLE)
        {
            // TODO: raise error
        }

        Value *v = static_cast<Table *>(table->param.value)->GetValue(key->param.value);
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
    }

    Table * VirtualMachine::GetKeyOwnerTable(const Value *key)
    {
        for (auto it = nest_tables_.rbegin(); it != nest_tables_.rend(); ++it)
            if ((*it)->HaveKey(key))
                return *it;
        return 0;
    }
} // namespace lua
