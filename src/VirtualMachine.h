#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <vector>

namespace lua
{
    class State;
    class Stack;
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
        void DoPush(Instruction *ins);

        State *state_;
        Stack *stack_;
        DataPool *data_pool_;

        typedef std::vector<Table *> NestTables;
        NestTables nest_tables_;
    };
} // namespace lua

#endif // VIRTUAL_MACHINE_H
