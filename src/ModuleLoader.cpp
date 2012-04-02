#include "ModuleLoader.h"
#include "State.h"

namespace lua
{
    ModuleLoader::ModuleInfo::ModuleInfo(const std::string& module_name, State *state)
        : source_(new Source(module_name.c_str())),
          parser_(new Parser(source_.get(), state)),
          boot_(new Bootstrap)
    {
        ParseTreeNodePtr root = parser_->Parse();
        InstructionSetWriter writer(boot_.get());

        Instruction *ins = writer.NewInstruction();
        ins->op_code = OpCode_Push;
        ins->param_a.type = InstructionParamType_Counter;
        ins->param_a.param.counter = 0;

        root->GenerateCode(&writer);

        ins = writer.NewInstruction();
        ins->op_code = OpCode_Call;
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

    Bootstrap * ModuleLoader::LoadModule(const std::string& module_name)
    {
        Modules::iterator it = modules_.find(module_name);
        if (it == modules_.end())
        {
            ModuleInfoPtr module(new ModuleInfo(module_name, state_));
            it = modules_.insert(modules_.begin(),
                std::make_pair(module_name, std::move(module)));
        }

        return it->second->boot_.get();
    }
} // namespace lua
