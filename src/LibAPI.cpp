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

    const char * StackAPI::GetCString(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->str_->GetCStr();
        else
            return "";
    }

    const String * StackAPI::GetString(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->str_;
        else
            return nullptr;
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

    UserData * StackAPI::GetUserData(int index)
    {
        Value *v = GetValue(index);
        if (v)
            return v->user_data_;
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

    void StackAPI::PushNil()
    {
        PushValue()->type_ = ValueT_Nil;
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

    void StackAPI::PushString(const std::string &str)
    {
        Value *v = PushValue();
        v->type_ = ValueT_String;
        v->str_ = state_->GetString(str);
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

    void StackAPI::PushUserData(UserData *user_data)
    {
        Value *v = PushValue();
        v->type_ = ValueT_UserData;
        v->user_data_ = user_data;
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

    void StackAPI::ArgCountError(int expect_count)
    {
        auto cfunc_error = state_->GetCFunctionErrorData();
        cfunc_error->type_ = CFuntionErrorType_ArgCount;
        cfunc_error->expect_arg_count_ = expect_count;
    }

    void StackAPI::ArgTypeError(int arg_index, ValueT expect_type)
    {
        auto cfunc_error = state_->GetCFunctionErrorData();
        cfunc_error->type_ = CFuntionErrorType_ArgType;
        cfunc_error->arg_index_ = arg_index;
        cfunc_error->expect_type_ = expect_type;
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
        RegisterFunc(global_, name, func);
    }

    void Library::RegisterTableFunction(const char *name, const TableMemberReg *table,
                                        std::size_t size)
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = state_->GetString(name);

        auto t = state_->NewTable();
        Value v;
        v.type_ = ValueT_Table;
        v.table_ = t;
        global_->SetValue(k, v);

        for (std::size_t i = 0; i < size; ++i)
        {
            switch (table[i].type_)
            {
                case ValueT_CFunction:
                    RegisterFunc(t, table[i].name_, table[i].func_);
                    break;
                case ValueT_Number:
                    RegisterNumber(t, table[i].name_, table[i].number_);
                    break;
                case ValueT_String:
                    RegisterString(t, table[i].name_, table[i].str_);
                    break;
                default: break;
            }
        }
    }

    void Library::RegisterFunc(Table *table, const char *name, CFunctionType func)
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = state_->GetString(name);

        Value v;
        v.type_ = ValueT_CFunction;
        v.cfunc_ = func;
        table->SetValue(k, v);
    }

    void Library::RegisterNumber(Table *table, const char *name, double number)
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = state_->GetString(name);

        Value v;
        v.type_ = ValueT_Number;
        v.num_ = number;
        table->SetValue(k, v);
    }

    void Library::RegisterString(Table *table, const char *name, const char *str)
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = state_->GetString(name);

        Value v;
        v.type_ = ValueT_String;
        v.str_ = state_->GetString(str);
        table->SetValue(k, v);
    }
} // namespace luna
