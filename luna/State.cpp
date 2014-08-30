#include "State.h"
#include "GC.h"
#include "VM.h"
#include "Lex.h"
#include "String.h"
#include "Function.h"
#include "Table.h"
#include "TextInStream.h"
#include "Exception.h"
#include <cassert>

namespace luna
{
#define METATABLES "__metatables"

    State::State()
    {
        module_manager_.reset(new ModuleManager(this));
        string_pool_.reset(new StringPool);

        // Init GC
        gc_.reset(new GC([&](GCObject *obj, unsigned int type) {
            if (type == GCObjectType_String)
            {
                string_pool_->DeleteString(static_cast<String *>(obj));
            }
            delete obj;
        }));
        auto root = std::bind(&State::FullGCRoot, this, std::placeholders::_1);
        gc_->SetRootTraveller(root, root);

        // New global table
        global_.table_ = NewTable();
        global_.type_ = ValueT_Table;

        // New table for store metatables
        Value k;
        k.type_ = ValueT_String;
        k.str_ = GetString(METATABLES);
        Value v;
        v.type_ = ValueT_Table;
        v.table_ = NewTable();
        global_.table_->SetValue(k, v);
    }

    State::~State()
    {
        gc_->ResetDeleter();
    }

    void State::AddModulePath(const std::string &path)
    {
        module_manager_->AddModulePath(path);
    }

    void State::LoadModule(const std::string &module_name)
    {
        module_manager_->LoadModule(module_name);
    }

    void State::DoModule(const std::string &module_name)
    {
        LoadModule(module_name);
        if (CallFunction(stack_.top_ - 1, 0, 0))
        {
            VM vm(this);
            vm.Execute();
        }
    }

    bool State::CallFunction(Value *f, int arg_count, int expect_result)
    {
        assert(f->type_ == ValueT_Closure || f->type_ == ValueT_CFunction);

        // Set stack top when arg_count is fixed
        if (arg_count != EXP_VALUE_COUNT_ANY)
            stack_.top_ = f + 1 + arg_count;

        if (f->type_ == ValueT_Closure)
        {
            // We need enter next ExecuteFrame
            CallClosure(f, expect_result);
            return true;
        }
        else
        {
            CallCFunction(f, expect_result);
            return false;
        }
    }

    String * State::GetString(const std::string &str)
    {
        auto s = string_pool_->GetString(str);
        if (!s)
        {
            s = gc_->NewString();
            s->SetValue(str);
            string_pool_->AddString(s);
        }
        return s;
    }

    String * State::GetString(const char *str, std::size_t len)
    {
        auto s = string_pool_->GetString(str, len);
        if (!s)
        {
            s = gc_->NewString();
            s->SetValue(str, len);
            string_pool_->AddString(s);
        }
        return s;
    }

    String * State::GetString(const char *str)
    {
        auto s = string_pool_->GetString(str);
        if (!s)
        {
            s = gc_->NewString();
            s->SetValue(str);
            string_pool_->AddString(s);
        }
        return s;
    }

    Function * State::NewFunction()
    {
        return gc_->NewFunction();
    }

    Closure * State::NewClosure()
    {
        return gc_->NewClosure();
    }

    Upvalue * State::NewUpvalue()
    {
        return gc_->NewUpvalue();
    }

    Table * State::NewTable()
    {
        return gc_->NewTable();
    }

    UserData * State::NewUserData()
    {
        return gc_->NewUserData();
    }

    CallInfo * State::GetCurrentCall()
    {
        if (calls_.empty())
            return nullptr;
        return &calls_.back();
    }

    Value * State::GetGlobal()
    {
        return &global_;
    }

    Table * State::GetMetatable(const char *metatable_name)
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = GetString(metatable_name);

        auto metatables = GetMetatables();
        auto metatable = metatables->GetValue(k);

        // Create table when metatable not existed
        if (metatable.type_ == ValueT_Nil)
        {
            metatable.type_ = ValueT_Table;
            metatable.table_ = NewTable();
            metatables->SetValue(k, metatable);
        }

        assert(metatable.type_ == ValueT_Table);
        return metatable.table_;
    }

    void State::EraseMetatable(const char *metatable_name)
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = GetString(metatable_name);

        Value nil;
        auto metatables = GetMetatables();
        metatables->SetValue(k, nil);
    }

    void State::FullGCRoot(GCObjectVisitor *v)
    {
        // Visit global table
        global_.Accept(v);

        // Visit stack values
        for (const auto &value : stack_.stack_)
        {
            value.Accept(v);
        }

        // Visit call info
        for (const auto &call : calls_)
        {
            call.register_->Accept(v);
            if (call.func_)
            {
                call.func_->Accept(v);
            }
        }
    }

    Table * State::GetMetatables()
    {
        Value k;
        k.type_ = ValueT_String;
        k.str_ = GetString(METATABLES);

        auto v = global_.table_->GetValue(k);
        assert(v.type_ == ValueT_Table);
        return v.table_;
    }

    void State::CallClosure(Value *f, int expect_result)
    {
        CallInfo callee;
        Function *callee_proto = f->closure_->GetPrototype();

        callee.func_ = f;
        callee.instruction_ = callee_proto->GetOpCodes();
        callee.end_ = callee.instruction_ + callee_proto->OpCodeSize();
        callee.expect_result_ = expect_result;

        Value *arg = f + 1;
        int fixed_args = callee_proto->FixedArgCount();

        // Fixed arg start from base register
        if (callee_proto->HasVararg())
        {
            Value *top = stack_.top_;
            callee.register_ = top;
            int count = top - arg;
            for (int i = 0; i < count && i < fixed_args; ++i)
                *top++ = *arg++;
        }
        else
        {
            callee.register_ = arg;
        }

        stack_.SetNewTop(callee.register_ + fixed_args);
        calls_.push_back(callee);
    }

    void State::CallCFunction(Value *f, int expect_result)
    {
        // Push the c function CallInfo
        CallInfo callee;
        callee.register_ = f + 1;
        callee.func_ = f;
        callee.expect_result_ = expect_result;
        calls_.push_back(callee);

        // Call c function
        CFunctionType cfunc = f->cfunc_;
        ClearCFunctionError();
        int res_count = cfunc(this);
        CheckCFunctionError();

        Value *src = nullptr;
        if (res_count > 0)
            src = stack_.top_ - res_count;

        // Copy c function result to caller stack
        Value *dst = f;
        if (expect_result == EXP_VALUE_COUNT_ANY)
        {
            for (int i = 0; i < res_count; ++i)
                *dst++ = *src++;
        }
        else
        {
            int count = std::min(expect_result, res_count);
            for (int i = 0; i < count; ++i)
                *dst++ = *src++;
            // Set all remain expect results to nil
            count = expect_result - res_count;
            for (int i = 0; i < count; ++i, ++dst)
                dst->SetNil();
        }

        // Set registers which after dst to nil
        // and set new stack top pointer
        stack_.SetNewTop(dst);

        // Pop the c function CallInfo
        calls_.pop_back();
    }

    void State::CheckCFunctionError()
    {
        auto error = GetCFunctionErrorData();
        if (error->type_ == CFuntionErrorType_NoError)
            return ;

        char buffer[128] = { 0 };
        if (error->type_ == CFuntionErrorType_ArgCount)
        {
            snprintf(buffer, sizeof(buffer), "expect %d arguments",
                     error->expect_arg_count_);
        }
        else if (error->type_ == CFuntionErrorType_ArgType)
        {
            auto &call = calls_.back();
            auto arg = call.register_ + error->arg_index_;
            snprintf(buffer, sizeof(buffer),
                     "argument #%d is a %s value, expect a %s value",
                     error->arg_index_ + 1, arg->TypeName(),
                     Value::TypeName(error->expect_type_));
        }

        // Pop the c function CallInfo
        calls_.pop_back();
        throw CallCFuncException(buffer);
    }
} // namespace luna
