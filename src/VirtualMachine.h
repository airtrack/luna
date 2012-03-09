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
    class Function;

    class VirtualMachine
    {
    public:
        VirtualMachine();
        void Init(State *state, Table *global_table);
        void Run(Function *f);

    private:
        void Assign();
        void CleanStack();
        void GetTable(String *name);

        State *state_;
        Stack *stack_;
        DataPool *data_pool_;

        typedef std::vector<Table *> NestTables;
        NestTables nest_tables_;
    };
} // namespace lua

#endif // VIRTUAL_MACHINE_H
