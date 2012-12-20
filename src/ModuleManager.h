#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include <string>

namespace luna
{
    class String;

    class ModuleManager
    {
    public:
        ModuleManager();

        ModuleManager(const ModuleManager&) = delete;
        void operator = (const ModuleManager&) = delete;

        void AddModulePath(const std::string &path);
        void LoadModule(const std::string &module_name);
        String *GetCurrentModule() const;
    };
} // namespace luna

#endif // MODULE_MANAGER_H
