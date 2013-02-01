#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include <string>

namespace luna
{
    class State;

    class ModuleManager
    {
    public:
        explicit ModuleManager(State *state);

        ModuleManager(const ModuleManager&) = delete;
        void operator = (const ModuleManager&) = delete;

        void AddModulePath(const std::string &path) { }
        void LoadModule(const std::string &module_name);

    private:
        State *state_;
    };
} // namespace luna

#endif // MODULE_MANAGER_H
