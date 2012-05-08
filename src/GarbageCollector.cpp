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
          locked_(false),
          total_bytes_(0),
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

        auto on_alloc = std::bind(&GarbageCollector::StatAlloc,
                                  this, std::placeholders::_1);
        auto on_dealloc = std::bind(&GarbageCollector::StatDealloc,
                                    this, std::placeholders::_1);

        data_pool_->SetOnAlloc(on_alloc);
        data_pool_->SetOnDealloc(on_dealloc);
    }

    void GarbageCollector::Run()
    {
        if (locked_)
            return ;

        // Mark all
        state_->GetGlobalTable()->Mark();
        module_loader_->MarkModules();
        vm_->MarkRuntime();

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

    void GarbageCollector::StatAlloc(std::size_t bytes)
    {
        total_bytes_ += bytes;
        ++total_count_;

        // We Start GC if GC is not locked
        if (total_count_ >= max_count_ && !locked_)
        {
            Run();

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
    }

    void GarbageCollector::StatDealloc(std::size_t bytes)
    {
        total_bytes_ -= bytes;
        --total_count_;
    }
} // namespace lua
