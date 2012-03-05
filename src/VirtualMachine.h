#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include "Instruction.h"

namespace lua
{
    class State;
    class Table;

    class VirtualMachine
    {
    public:
        VirtualMachine();
        void Init(State *state, Table *global_table);

    private:
        State *state_;
        Table *global_table_;
    };
} // namespace lua

#endif // VIRTUAL_MACHINE_H
