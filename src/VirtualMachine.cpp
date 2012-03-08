#include "VirtualMachine.h"
#include "State.h"
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

    void VirtualMachine::Init(State *state, Table *global_table)
    {
        state_ = state;
        stack_ = state->GetStack();
        data_pool_ = state->GetDataPool();
        nest_tables_.push_back(global_table);
    }

    void VirtualMachine::Run(Function *f)
    {
        nest_tables_.push_back(data_pool_->GetTable());
        Instruction *instructions = f->GetInstructions();
        std::size_t ins_count = f->GetInstructionCount();
        std::size_t current = 0;

        while (current < ins_count)
        {
            Instruction *ins = &instructions[current];
            switch (ins->op_code)
            {
            case OpCode_Assign:
                break;
            case OpCode_ClearResult:
                break;
            case OpCode_GetTable:
                assert(ins->param_a.type == InstructionParamType_Name);
                GetTable(ins->param_a.param.name);
                break;
            case OpCode_Push:
                if (ins->param_a.type == InstructionParamType_Name)
                    stack_->Push(ins->param_a.param.name);
                break;
            }
            ++current;
        }
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
                return ;
            }
        }

        // If can not find key name from all tables, then we use global table.
        stack_->Push(nest_tables_.front());
    }
} // namespace lua
