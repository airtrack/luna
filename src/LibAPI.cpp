#include "LibAPI.h"
#include "State.h"
#include "Runtime.h"
#include <assert.h>

namespace luna
{
    LibAPI::LibAPI(State *state)
        : state_(state),
          stack_(&state->stack_)
    {
    }

    ValueT LibAPI::GetValueType(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->type_;
        else
            return ValueT_Nil;
    }

    double LibAPI::GetNumber(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->num_;
        else
            return 0.0;
    }

    const char * LibAPI::GetString(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->str_->GetStdString().c_str();
        else
            return "";
    }

    void LibAPI::PushNumber(double num)
    {
        Value *v = PushValue();
        v->type_ = ValueT_Number;
        v->num_ = num;
    }

    void LibAPI::PushString(const char *string)
    {
        Value *v = PushValue();
        v->type_ = ValueT_String;
        v->str_ = state_->GetString(string);
    }

    Value * LibAPI::GetValue(int index)
    {
        assert(!state_->calls_.empty());
        Value *v = nullptr;
        if (index < 0)
            v = stack_->top_ + index;
        else
            v = state_->calls_.back().register_ + index;

        if (v >= stack_->top_ || v < state_->calls_.back().register_)
            return nullptr;
        else
            return v;
    }

    Value * LibAPI::PushValue()
    {
        return stack_->top_++;
    }
} // namespace luna
