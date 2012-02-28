#ifndef STATE_H
#define STATE_H

#include "DataPool.h"
#include "types/Table.h"

namespace lua
{
    class State
    {
    public:
        State();
        DataPool * GetDataPool();

    private:
        DataPool *data_pool_;
        Table *global_table_;
    };
} // namespace lua

#endif // STATE_H
