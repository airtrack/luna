#ifndef STATE_H
#define STATE_H

#include "DataPool.h"
#include "ModuleLoader.h"
#include "VirtualMachine.h"
#include "types/Table.h"
#include <memory>

namespace lua
{
    class State
    {
    public:
        State();
        DataPool * GetDataPool();
        ModuleLoader * GetModuleLoader();
        VirtualMachine * GetVM();

    private:
        std::unique_ptr<DataPool> data_pool_;
        std::unique_ptr<ModuleLoader> module_loader_;
        std::unique_ptr<VirtualMachine> vm_;
        Table *global_table_;
    };
} // namespace lua

#endif // STATE_H
