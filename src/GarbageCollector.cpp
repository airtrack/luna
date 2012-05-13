#include "GarbageCollector.h"
#include "State.h"
#include <functional>

namespace lua
{
    GarbageCollector::GarbageCollector()
        : state_(0),
          stack_(0),
          data_pool_(0),
          module_loader_(0),
          vm_(0),
          need_run_(false),
          total_count_(0),
          max_count_(kBaseMaxCount)
    {
    }

    void GarbageCollector::Init(State *state)
    {
        state_ = state;
        stack_ = state->GetStack();
        data_pool_ = state->GetDataPool();
        module_loader_ = state->GetModuleLoader();
        vm_ = state->GetVM();

        auto on_alloc = std::bind(&GarbageCollector::StatAlloc, this);
        auto on_dealloc = std::bind(&GarbageCollector::StatDealloc, this);

        data_pool_->SetOnAlloc(on_alloc);
        data_pool_->SetOnDealloc(on_dealloc);
    }

    void GarbageCollector::Run()
    {
        // Mark all
        state_->GetGlobalTable()->Mark();
        module_loader_->MarkModules();
        vm_->MarkRuntime();

        // Sweep garbage
        data_pool_->Sweep();

        AfterRun();
    }

    void GarbageCollector::AfterRun()
    {
        need_run_ = false;

        if (total_count_ >= (max_count_ / 5 * 4))
        {
            // If there are a few objects has been sweep,
            // we increase max_count_
            while (max_count_ / total_count_ < 2)
                max_count_ *= 2;
        }
        else if (total_count_ < (max_count_ / 5 * 2))
        {
            // If there are a lot objects has been sweep,
            // we decrease max_count_
            while (total_count_ < (max_count_ / 5 * 2))
                max_count_ /= 2;
            max_count_ = max_count_ > kBaseMaxCount ? max_count_ : kBaseMaxCount;
        }
    }

    void GarbageCollector::StatAlloc()
    {
        ++total_count_;

        // We mark need_run_
        if (total_count_ >= max_count_)
            need_run_ = true;
    }

    void GarbageCollector::StatDealloc()
    {
        --total_count_;
    }
} // namespace lua
