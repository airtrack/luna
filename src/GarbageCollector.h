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

        // If GC lock, then GC will not work,
        // GC resume work until unlock.
        void Lock();
        void Unlock();

    private:
        void StatAlloc(std::size_t bytes);
        void StatDealloc(std::size_t bytes);

        static const std::size_t kBaseMaxCount = 2048;

        State *state_;
        Stack *stack_;
        DataPool *data_pool_;
        ModuleLoader *module_loader_;
        VirtualMachine *vm_;

        bool locked_;

        std::size_t total_bytes_;
        std::size_t total_count_;
        std::size_t max_count_;
    };

    class GCLocker
    {
    public:
        explicit GCLocker(GarbageCollector *gc)
            : gc_(gc)
        {
            gc_->Lock();
        }

        ~GCLocker()
        {
            gc_->Unlock();
        }

    private:
        GarbageCollector *gc_;
    };
} // namespace lua

#endif // GARBAGE_COLLECTOR_H
