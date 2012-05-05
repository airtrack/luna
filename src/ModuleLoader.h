#ifndef MODULE_LOADER_H
#define MODULE_LOADER_H

#include "Source.h"
#include "Parser.h"
#include "Bootstrap.h"
#include <map>
#include <memory>
#include <string>

namespace lua
{
    class State;

    class ModuleLoader
    {
    public:
        ModuleLoader();
        void Init(State *state);
        bool IsModuleLoaded(const std::string& module_name) const;
        Bootstrap* LoadModule(const std::string& module_name);
        void MarkModules();

    private:
        struct ModuleInfo
        {
            std::unique_ptr<Source> source_;
            std::unique_ptr<Parser> parser_;
            std::unique_ptr<Bootstrap> boot_;

            ModuleInfo(const std::string& module_name, State *state);
        };

        typedef std::unique_ptr<ModuleInfo> ModuleInfoPtr;
        typedef std::map<std::string, ModuleInfoPtr> Modules;

        Modules modules_;
        State *state_;
    };
} // namespace lua

#endif // MODULE_LOADER_H
