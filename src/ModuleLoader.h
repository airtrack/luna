#ifndef MODULE_LOADER_H
#define MODULE_LOADER_H

#include <map>
#include <memory>
#include <string>

namespace lua
{
    class State;
    class Source;
    class Parser;
    class Function;

    class ModuleLoader
    {
    public:
        ModuleLoader();
        void Init(State *state);
        bool IsModuleLoaded(const std::string& module_name) const;
        Function* LoadModule(const std::string& module_name);

    private:
        struct ModuleInfo
        {
            std::unique_ptr<Source> source_;
            std::unique_ptr<Parser> parser_;
            Function *chunk_;                   // Construct from DataPool

            ModuleInfo(const std::string& module_name, State *state);
        };

        typedef std::unique_ptr<ModuleInfo> ModuleInfoPtr;
        typedef std::map<std::string, ModuleInfoPtr> Modules;

        Modules modules_;
        State *state_;
    };
} // namespace lua

#endif // MODULE_LOADER_H
