#include "VirtualMachine.h"

namespace lua
{
    VirtualMachine::VirtualMachine()
        : state_(0),
          global_table_(0)
    {
    }

    void VirtualMachine::Init(State *state, Table *global_table)
    {
        state_ = state;
        global_table_ = global_table;
    }
} // namespace lua
