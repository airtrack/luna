#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H

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
        State *state_;
        Stack *stack_;
        DataPool *data_pool_;
        ModuleLoader *module_loader_;
        VirtualMachine *vm_;

        bool locked_;
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
