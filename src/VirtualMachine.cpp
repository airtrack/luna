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

        Table *upvalue_table = cl->GetUpvalueTable();
        if (upvalue_table)
        {
            const NameSet *upvalue_set = func->GetUpValueSet();
        }
    }

    void VirtualMachine::Return()
    {
    }

    void VirtualMachine::GenerateArgTable()
    {
    }

    void VirtualMachine::ReserveStack()
    {
    }

    void VirtualMachine::ExtendCounter()
    {
    }

    void VirtualMachine::MergeCounter()
    {
    }

    void VirtualMachine::Call()
    {
    }
} // namespace lua
