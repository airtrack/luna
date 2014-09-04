#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include "Value.h"
#include <string>

namespace luna
{
    class State;
    class Lexer;

    // Load and manage all modules or load string
    class ModuleManager
    {
    public:
        ModuleManager(State *state, Table *modules);

        ModuleManager(const ModuleManager&) = delete;
        void operator = (const ModuleManager&) = delete;

        // Check module loaded or not
        bool IsLoaded(const std::string &module_name) const;

        // Get module closure when module loaded,
        // if the module is not loaded, return nil value
        Value GetModuleClosure(const std::string &module_name) const;

        // Load module, when loaded success, push the closure of the module
        // onto stack
        void LoadModule(const std::string &module_name);

        // Load string, when loaded success, push the closure of the string
        // onto stack
        void LoadString(const std::string &str, const std::string &name);

    private:
        // Load and push the closure onto stack
        void Load(Lexer &lexer);

        State *state_;
        Table *modules_;
    };
} // namespace luna

#endif // MODULE_MANAGER_H
