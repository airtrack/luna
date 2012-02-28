#include "ModuleLoader.h"
#include "State.h"
#include "Source.h"
#include "Parser.h"
#include "FunctionWriterAdaptor.h"

namespace lua
{
    ModuleLoader::ModuleInfo::ModuleInfo(const std::string& module_name, State *state)
        : source_(new Source(module_name.c_str())),
          parser_(new Parser(source_.get())),
          chunk_(0)
    {
        ParseTreeNodePtr root = parser_->Parse();
        chunk_ = state->GetDataPool()->GetFunction();

        FunctionWriterAdaptor writer(chunk_);
        root->GenerateCode(&writer);
    }

    ModuleLoader::ModuleLoader()
        : state_(0)
    {
    }

    void ModuleLoader::Init(State *state)
    {
        state_ = state;
    }

    bool ModuleLoader::IsModuleLoaded(const std::string& module_name) const
    {
        return modules_.find(module_name) != modules_.end();
    }

    Function * ModuleLoader::LoadModule(const std::string& module_name)
    {
        Modules::iterator it = modules_.find(module_name);
        if (it == modules_.end())
        {
            ModuleInfoPtr module(new ModuleInfo(module_name, state_));
            it = modules_.insert(modules_.begin(),
                std::make_pair(module_name, std::move(module)));
        }

        return it->second->chunk_;
    }
} // namespace lua
