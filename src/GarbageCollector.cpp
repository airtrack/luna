#include "GarbageCollector.h"
#include "State.h"

namespace lua
{
    GarbageCollector::GarbageCollector()
        : state_(0),
          stack_(0),
          data_pool_(0),
          module_loader_(0),
          vm_(0),
          locked_(false)
    {
    }

    void GarbageCollector::Init(State *state)
    {
        state_ = state;
        stack_ = state->GetStack();
        data_pool_ = state->GetDataPool();
        module_loader_ = state->GetModuleLoader();
        vm_ = state->GetVM();
    }

    void GarbageCollector::Run()
    {
        if (locked_)
            return ;

        // Mark all
        state_->GetGlobalTable()->Mark();
        stack_->MarkStackValues();
        module_loader_->MarkModules();
        vm_->MarkLocalTables();

        // Sweep garbage
        data_pool_->Sweep();
    }

    void GarbageCollector::Lock()
    {
        locked_ = true;
    }

    void GarbageCollector::Unlock()
    {
        locked_ = false;
    }
} // namespace lua
