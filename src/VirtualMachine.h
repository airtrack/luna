#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <vector>

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
        void GetTable(String *name);
        void GetLocalTable();
        void GetTableValue();
        void DoPush(Instruction *ins);
        void GenerateClosure(Instruction *ins);
        void Return();
        void GenerateArgTable();
        void ReserveStack();
        void ExtendCounter();
        void MergeCounter();
        void Call();

        Table * GetKeyOwnerTable(const Value *key);

        State *state_;
        Stack *stack_;
        DataPool *data_pool_;

        typedef std::vector<Table *> NestTables;
        NestTables nest_tables_;
    };
} // namespace lua

#endif // VIRTUAL_MACHINE_H
