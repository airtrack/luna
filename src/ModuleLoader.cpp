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
        CodeWriter writer(boot_.get());

        Instruction *ins = writer.NewInstruction();
        ins->op_code = OpCode_AddGlobalTable;

        root->GenerateCode(&writer);

        // Push params
        ins = writer.NewInstruction();
        ins->op_code = OpCode_Push;
        ins->param_a.type = InstructionParamType_Counter;
        ins->param_a.param.counter = 0;

        ins = writer.NewInstruction();
        ins->op_code = OpCode_Call;

        // Clear return values
        ins = writer.NewInstruction();
        ins->op_code = OpCode_CleanStack;

        ins = writer.NewInstruction();
        ins->op_code = OpCode_DelGlobalTable;
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
            // Lock GC when load new module
            GCLocker locker(state_->GetGC());

            ModuleInfoPtr module(new ModuleInfo(module_name, state_));
            it = modules_.insert(modules_.begin(),
                std::make_pair(module_name, std::move(module)));
        }

        return it->second->boot_.get();
    }

    void ModuleLoader::MarkModules()
    {
        for (auto it = modules_.begin(); it != modules_.end(); ++it)
            it->second->boot_->MarkInstructions();
    }
} // namespace lua
