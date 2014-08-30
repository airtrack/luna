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

    // Error type reported by called c function
    enum CFuntionErrorType
    {
        CFuntionErrorType_NoError,
        CFuntionErrorType_ArgCount,
        CFuntionErrorType_ArgType,
    };

    // Error reported by called c function
    struct CFunctionError
    {
        CFuntionErrorType type_;
        union
        {
            int expect_arg_count_;
            struct
            {
                int arg_index_;
                ValueT expect_type_;
            };
        };

        CFunctionError() : type_(CFuntionErrorType_NoError) { }
    };

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

        // Call an in stack function
        // If f is a closure, then create a stack frame and return true,
        // call VM::Execute() to execute the closure instructions.
        // Return false when f is a c function.
        bool CallFunction(Value *f, int arg_count, int expect_result);

        // New GCObjects
        String * GetString(const std::string &str);
        String * GetString(const char *str, std::size_t len);
        String * GetString(const char *str);
        Function * NewFunction();
        Closure * NewClosure();
        Upvalue * NewUpvalue();
        Table * NewTable();
        UserData * NewUserData();

        // Get current CallInfo
        CallInfo * GetCurrentCall();

        // Get global table value
        Value * GetGlobal();

        // Metatable operations
        // Return metatable, create when metatable not existed
        Table * GetMetatable(const char *metatable_name);
        // Erase metatable
        void EraseMetatable(const char *metatable_name);

        // For call c function
        void ClearCFunctionError()
        { cfunc_error_.type_ = CFuntionErrorType_NoError; }

        // Error data for call c function
        CFunctionError * GetCFunctionErrorData()
        { return &cfunc_error_; }

        // Get the GC
        GC& GetGC()
        { return *gc_; }

        // Check and run GC
        void CheckRunGC()
        { gc_->CheckGC(); }

    private:
        // Full GC root
        void FullGCRoot(GCObjectVisitor *v);

        void CallClosure(Value *f, int expect_result);
        void CallCFunction(Value *f, int expect_result);
        void CheckCFunctionError();

        Table * GetMetatables();

        std::unique_ptr<ModuleManager> module_manager_;
        std::unique_ptr<StringPool> string_pool_;
        std::unique_ptr<GC> gc_;

        // For c function error
        CFunctionError cfunc_error_;

        // For VM
        Stack stack_;
        std::list<CallInfo> calls_;
        Value global_;
    };
} // namespace luna

#endif // STATE_H
