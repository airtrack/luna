#include "LibAPI.h"
#include "State.h"
#include "Runtime.h"
#include "Table.h"
#include <assert.h>

namespace luna
{
    StackAPI::StackAPI(State *state)
        : state_(state),
          stack_(&state->stack_)
    {
    }

    int StackAPI::GetStackSize() const
    {
        return stack_->top_ - state_->calls_.back().register_;
    }

    ValueT StackAPI::GetValueType(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->type_;
        else
            return ValueT_Nil;
    }

    double StackAPI::GetNumber(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->num_;
        else
            return 0.0;
    }

    const char * StackAPI::GetString(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->str_->GetCStr();
        else
            return "";
    }

    bool StackAPI::GetBool(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->bvalue_;
        else
            return false;
    }

    Closure * StackAPI::GetClosure(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->closure_;
        else
            return nullptr;
    }

    Table * StackAPI::GetTable(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->table_;
        else
            return nullptr;
    }

    CFunctionType StackAPI::GetCFunction(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->cfunc_;
        else
            return nullptr;
    }

    Value * StackAPI::GetValue(int index)
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
    
    void StackAPI::PushNumber(double num)
    {
        Value *v = PushValue();
        v->type_ = ValueT_Number;
        v->num_ = num;
    }

    void StackAPI::PushString(const char *string)
    {
        Value *v = PushValue();
        v->type_ = ValueT_String;
        v->str_ = state_->GetString(string);
    }

    void StackAPI::PushBool(bool value)
    {
        Value *v = PushValue();
        v->type_ = ValueT_Bool;
        v->bvalue_ = value;
    }

    void StackAPI::PushTable(Table *table)
    {
        Value *v = PushValue();
        v->type_ = ValueT_Table;
        v->table_ = table;
    }

    void StackAPI::PushCFunction(CFunctionType function)
    {
        Value *v = PushValue();
        v->type_ = ValueT_CFunction;
        v->cfunc_ = function;
    }

    void StackAPI::PushValue(const Value &value)
    {
        *PushValue() = value;
    }

    Value * StackAPI::PushValue()
    {
        return stack_->top_++;
    }

    Library::Library(State *state)
        : state_(state),
          global_(state->global_.table_)
    {
    }

    void Library::RegisterFunc(const char *name, CFunctionType func)
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = state_->GetString(name);

        Value v;
        v.type_ = ValueT_CFunction;
        v.cfunc_ = func;
        global_->SetValue(k, v);
    }
} // namespace luna
