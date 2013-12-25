#ifndef STATE_H
#define STATE_H

#include "GC.h"
#include "Runtime.h"
#include "ModuleManager.h"
#include "StringPool.h"
#include <string>
#include <memory>
#include <vector>
#include <list>

namespace luna
{
    class VM;

    class State
    {
        friend class VM;
        friend class StackAPI;
        friend class Library;
        friend class Bootstrap;
        friend class CodeGenerateVisitor;
    public:
        State();
        ~State();

        State(const State&) = delete;
        void operator = (const State&) = delete;

        // Add module search path
        void AddModulePath(const std::string &path);

        // Load modules
        void LoadModule(const std::string &module_name);
        void LoadString(const std::string &script_str);

        // New GCObjects
        String * GetString(const std::string &str);
        String * GetString(const char *str, std::size_t len);
        String * GetString(const char *str);
        Function * NewFunction();
        Closure * NewClosure();
        Upvalue * NewUpvalue();
        Table * NewTable();

        // Get current CallInfo
        CallInfo * GetCurrentCall();

        // Get global table value
        Value * GetGlobal();

    private:
        std::unique_ptr<ModuleManager> module_manager_;
        std::unique_ptr<StringPool> string_pool_;
        std::unique_ptr<GC> gc_;

        // for VM
        Stack stack_;
        std::list<CallInfo> calls_;
        Value global_;
    };
} // namespace luna

#endif // STATE_H
