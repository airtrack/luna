#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <vector>
#include <memory>

namespace lua
{
    class State;
    class Stack;
    class Value;
    class Table;
    class DataPool;
    class String;
    class Bootstrap;
    struct Instruction;

    class VirtualMachine
    {
    public:
        VirtualMachine();
        void Init(State *state);
        void Run(Bootstrap *boot);

    private:
        void Assign();
        void CleanStack();
        void GetTable(Instruction *ins);
        void GetLocalTable();
        void GetTableValue(Instruction *ins);
        void DoPush(Instruction *ins);
        void GenerateClosure(Instruction *ins);
        void Return();
        void GenerateArgTable();
        void MergeCounter();
        void ResetCounter();
        void DuplicateCounter();
        void Call();
        void AddLocalTable();
        void DelLocalTable();
        void AddGlobalTable();
        void DelGlobalTable();
        void Power();
        void Multiply();
        void Divide();
        void Mod();
        void Plus();
        void Minus();
        void Concat();
        void Less();
        void Greater();
        void LessEqual();
        void GreaterEqual();

        void CheckOperand(double& left, double& right);
        void SetOperResult(double result);
        Table * GetUpvalueKeyOwnerTable(const Value *key);

        struct CallStackInfo
        {
            Instruction *caller_base;
            std::size_t caller_total;
            std::size_t caller_offset;
            Value *callee;
            std::size_t callee_tables;

            CallStackInfo(Instruction *caller_b, std::size_t caller_t,
                          std::size_t caller_o, Value *c_ee)
                : caller_base(caller_b),
                  caller_total(caller_t),
                  caller_offset(caller_o),
                  callee(c_ee),
                  callee_tables(0)
            {
            }
        };

        State *state_;
        Stack *stack_;
        DataPool *data_pool_;

        typedef std::vector<Table *> NestTables;
        NestTables nest_tables_;

        typedef std::vector<CallStackInfo> CallStack;
        CallStack call_stack_;

        Instruction *ins_base_;
        int ins_count_;
        int ins_current_;

        std::unique_ptr<Bootstrap> native_func_ret_;
    };
} // namespace lua

#endif // VIRTUAL_MACHINE_H
