#include "State.h"
#include "Lex.h"
#include "GCObject.h"
#include "String.h"
#include "Function.h"
#include "Table.h"
#include "TextInStream.h"

namespace luna
{
    State::State()
        : string_pool_(new StringPool)
    {
        module_manager_.reset(new ModuleManager(this));
        global_.table_ = NewTable();
        global_.type_ = ValueT_Table;
    }

    State::~State()
    {
        for (auto o : gclist_)
            delete o;
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
        return string_pool_->AllocString(str);
    }

    Function * State::NewFunction()
    {
        auto f = new Function;
        gclist_.push_back(f);
        return f;
    }

    Closure * State::NewClosure()
    {
        auto c = new Closure;
        gclist_.push_back(c);
        return c;
    }

    Table * State::NewTable()
    {
        auto t = new Table;
        gclist_.push_back(t);
        return t;
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
