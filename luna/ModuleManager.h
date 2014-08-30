#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include "Value.h"
#include <string>

namespace luna
{
    class State;

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

        // Load module, when loaded success, push the module
        // closure on stack
        void LoadModule(const std::string &module_name);

    private:
        State *state_;
        Table *modules_;
    };
} // namespace luna

#endif // MODULE_MANAGER_H
