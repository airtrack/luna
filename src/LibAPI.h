#ifndef LIB_API_H
#define LIB_API_H

#include "Value.h"
#include <string>

namespace luna
{
    struct Stack;
    class State;
    class Table;
    class Closure;

    // This class is API for library to manipulate stack,
    // stack index value is:
    // -1 ~ -n is top to bottom,
    // 0 ~ n is bottom to top.
    class StackAPI
    {
    public:
        explicit StackAPI(State *state);

        // Helper functions for check API arguments
        // e.g.
        //   bool result = CheckArgs(2, ValueT_String, ValueT_Number);
        //   2: min arguments to call API
        //   ValueT_String: type of the first argument
        //   ValueT_Number: type of the second argument
        //   all arguments are valid when result == true
        bool CheckArgs(int index, int params)
        {
            // No more expect argument to check, success
            return true;
        }

        template<typename... ValueTypes>
        bool CheckArgs(int index, int params, ValueT type, ValueTypes... types)
        {
            // All arguments check success
            if (index == params)
                return true;

            // Check type of the index + 1 argument
            if (GetValueType(index) != type)
            {
                ArgTypeError(index, type);
                return false;
            }

            // Check remain arguments
            return CheckArgs(++index, params, types...);
        }

        template<typename... ValueTypes>
        bool CheckArgs(int minCount, ValueTypes... types)
        {
            // Check count of arguments
            auto params = GetStackSize();
            if (params < minCount)
            {
                ArgCountError(minCount);
                return false;
            }

            return CheckArgs(0, params, types...);
        }

        // Get count of value in this function stack
        int GetStackSize() const;

        // Get value type by index of stack
        ValueT GetValueType(int index);

        // Check value type by index of stack
        bool IsNumber(int index) { return GetValueType(index) == ValueT_Number; }
        bool IsString(int index) { return GetValueType(index) == ValueT_String; }
        bool IsBool(int index) { return GetValueType(index) == ValueT_Bool; }
        bool IsClosure(int index) { return GetValueType(index) == ValueT_Closure; }
        bool IsTable(int index) { return GetValueType(index) == ValueT_Table; }
        bool IsCFunction(int index) { return GetValueType(index) == ValueT_CFunction; }

        // Get value from stack by index
        double GetNumber(int index);
        const char * GetCString(int index);
        const String * GetString(int index);
        bool GetBool(int index);
        Closure * GetClosure(int index);
        Table * GetTable(int index);
        CFunctionType GetCFunction(int index);
        Value * GetValue(int index);

        // Push value to stack
        void PushNil();
        void PushNumber(double num);
        void PushString(const char *string);
        void PushString(const std::string &str);
        void PushBool(bool value);
        void PushTable(Table *table);
        void PushCFunction(CFunctionType function);
        void PushValue(const Value &value);

        // For report argument error
        void ArgCountError(int expect_count);
        void ArgTypeError(int arg_index, ValueT expect_type);

    private:
        // Push value to stack, and return the value
        Value * PushValue();

        State *state_;
        Stack *stack_;
    };

    // For register table functions
    struct TableFuncReg
    {
        const char *name_;
        CFunctionType func_;
    };

    // This class register C function to luna
    class Library
    {
    public:
        explicit Library(State *state);

        // Register global function 'func' as 'name'
        void RegisterFunc(const char *name, CFunctionType func);

        // Register a table of functions
        void RegisterTableFunction(const char *name, const TableFuncReg *table,
                                   std::size_t size);

        template<std::size_t N>
        void RegisterTableFunction(const char *name, const TableFuncReg (&table)[N])
        {
            RegisterTableFunction(name, table, N);
        }

    private:
        void RegisterFunc(Table *table, const char *name, CFunctionType func);

        State *state_;
        Table *global_;
    };
} // namespace luna

#endif // LIB_API_H
