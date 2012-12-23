#include "State.h"
#include "Lex.h"
#include "String.h"
#include "TextInStream.h"
#include "ModuleManager.h"

namespace luna
{
    State::State()
        : module_manager_(new ModuleManager)
    {
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
        return new String(str);
    }
} // namespace luna
