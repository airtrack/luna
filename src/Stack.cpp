#include "Stack.h"
#include <assert.h>

namespace lua
{
    Stack::Stack(int base_stack_size)
    {
        stack_.reserve(base_stack_size);
    }

    std::size_t Stack::Size() const
    {
        return stack_.size();
    }

    StackValue * Stack::GetStackValue(int index)
    {
        int size = stack_.size();
        if (index >= 0)
        {
            if (index < size)
                return &stack_[index];
        }
        else
        {
            index += size;
            if (index >= 0)
                return &stack_[index];
        }

        return 0;
    }

    const StackValue * Stack::GetStackValue(int index) const
    {
        int size = stack_.size();
        if (index >= 0)
        {
            if (index < size)
                return &stack_[index];
        }
        else
        {
            index += size;
            if (index >= 0)
                return &stack_[index];
        }

        return 0;
    }

    StackValue * Stack::Top()
    {
        if (stack_.empty())
            return 0;
        return &stack_.back();
    }

    const StackValue * Stack::Top() const
    {
        if (stack_.empty())
            return 0;
        return &stack_.back();
    }

    void Stack::Pop(int count)
    {
        assert(count > 0);
        int remain = static_cast<int>(stack_.size()) - count;
        remain = remain < 0 ? 0 : remain;
        stack_.resize(remain);
    }

    void Stack::Clear()
    {
        stack_.clear();
    }

    StackValue * Stack::Push()
    {
        stack_.resize(stack_.size() + 1);
        return Top();
    }

    void Stack::Push(Value *value)
    {
        StackValue *sv = Push();
        sv->type = StackValueType_Value;
        sv->param.value = value;
    }

    void Stack::Push(int total, int current)
    {
        StackValue *sv = Push();
        sv->type = StackValueType_Counter;
        sv->param.counter.total = total;
        sv->param.counter.current = current;
    }
} // namespace lua
