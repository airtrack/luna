#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H

#include <cstddef>

namespace lua
{
    class State;
    class Stack;
    class DataPool;
    class ModuleLoader;
    class VirtualMachine;

    class GarbageCollector
    {
    public:
        GarbageCollector();

        void Init(State *state);

        // Run the GC if GC not locked.
        void Run();

        // Check GC need run to collect garbage.
        bool NeedRun() const
        {
            return need_run_;
        }

    private:
        void AfterRun();
        void StatAlloc();
        void StatDealloc();

        static const std::size_t kBaseMaxCount = 2048;

        State *state_;
        Stack *stack_;
        DataPool *data_pool_;
        ModuleLoader *module_loader_;
        VirtualMachine *vm_;

        bool need_run_;

        std::size_t total_count_;
        std::size_t max_count_;
    };
} // namespace lua

#endif // GARBAGE_COLLECTOR_H
