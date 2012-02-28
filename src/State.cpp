#include "State.h"

namespace lua
{
    State::State()
        : data_pool_(0),
          global_table_(0)
    {
    }

    DataPool * State::GetDataPool()
    {
        return data_pool_;
    }
} // namespace lua
