#ifndef STATE_H
#define STATE_H

#include "Stack.h"
#include "DataPool.h"
#include "ModuleLoader.h"
#include "VirtualMachine.h"
#include "GarbageCollector.h"
#include "types/Table.h"
#include <memory>

namespace lua
{
    class State
    {
    public:
        State();
        Stack * GetStack();
        DataPool * GetDataPool();
        ModuleLoader * GetModuleLoader();
        VirtualMachine * GetVM();
        GarbageCollector * GetGC();
        Table * GetGlobalTable();

    private:
        std::unique_ptr<Stack> stack_;
        std::unique_ptr<DataPool> data_pool_;
        std::unique_ptr<ModuleLoader> module_loader_;
        std::unique_ptr<VirtualMachine> vm_;
        std::unique_ptr<GarbageCollector> gc_;
        Table *global_table_;
    };
} // namespace lua

#endif // STATE_H
