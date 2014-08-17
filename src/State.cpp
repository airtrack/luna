#include "State.h"
#include "Lex.h"
#include "GC.h"
#include "String.h"
#include "Function.h"
#include "Table.h"
#include "TextInStream.h"
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

    void State::LoadString(const std::string &script_str)
    {
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
} // namespace luna
