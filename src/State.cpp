#include "State.h"
#include "Lex.h"
#include "GC.h"
#include "String.h"
#include "Function.h"
#include "Table.h"
#include "TextInStream.h"

namespace luna
{
    State::State()
    {
        module_manager_.reset(new ModuleManager(this));
        string_pool_.reset(new StringPool);

        gc_.reset(new GC([&](GCObject *obj, unsigned int type) {
            if (type == GCObjectType_String)
                string_pool_->DeleteString(static_cast<String *>(obj));
        }));

        // New global table
        global_.table_ = NewTable();
        global_.type_ = ValueT_Table;
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
} // namespace luna
