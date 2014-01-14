#ifndef FUNCTION_H
#define FUNCTION_H

#include "GC.h"
#include "Value.h"
#include "OpCode.h"
#include "String.h"
#include "Upvalue.h"
#include <vector>

namespace luna
{
    // Function prototype class, all runtime functions(closures) reference this
    // class object. This class contains some static information generated after
    // parse.
    class Function : public GCObject
    {
    public:
        struct UpvalueInfo
        {
            // Upvalue name
            String *name_;

            // This upvalue is parent function's local variable
            // when value is true, otherwise it is parent parent
            // (... and so on) function's local variable
            bool parent_local_;

            // Register id when this upvalue is parent function's
            // local variable, otherwise it is index of upvalue list
            // of parent function
            int register_index_;

            UpvalueInfo(String *name, bool parent_local,
                        int register_index)
            : name_(name), parent_local_(parent_local),
            register_index_(register_index) { }
        };

        Function();

        virtual void Accept(GCObjectVisitor *v);

        // Get function instructions and size
        const Instruction * GetOpCodes() const;
        std::size_t OpCodeSize() const;
        // Get instruction pointer, then it can be changed
        Instruction * GetMutableInstruction(std::size_t index);

        // Add instruction, 'line' is line number of the instruction 'i',
        // return index of the new instruction
        std::size_t AddInstruction(Instruction i, int line);

        // Set and get this function has vararg
        void SetHasVararg();
        bool HasVararg() const;

        // Set and get fixed arg count
        void SetFixedArgCount(int count);
        int FixedArgCount() const;

        // Set module and function define start line
        void SetModuleName(String *module);
        void SetLine(int line);

        // Set superior function
        void SetSuperior(Function *superior);

        // Add const number and return index of the const value
        int AddConstNumber(double num);

        // Add const String and return index of the const value
        int AddConstString(String *str);

        // Add const Value and return index of the const value
        int AddConstValue(const Value &v);

        // Add local variable debug info
        void AddLocalVar(String *name, int register_id,
                         int begin_pc, int end_pc);

        // Add child function, return index of the function
        int AddChildFunction(Function *child);

        // Add a upvalue, return index of the upvalue
        int AddUpvalue(String *name, bool parent_local, int register_index);

        // Get upvalue index when the name upvalue existed, otherwise return -1
        int SearchUpvalue(String *name) const;

        // Get child function by index
        Function * GetChildFunction(int index) const;

        // Search local variable name from local variable list
        String * SearchLocalVar(int register_id, int pc) const;

        // Get const Value by index
        Value * GetConstValue(int i);

        // Get instruction line by instruction index
        int GetInstructionLine(int i) const;

        // Get upvalue count
        std::size_t GetUpvalueCount() const
        { return upvalues_.size(); }

        // Get upvalue info by index
        const UpvalueInfo * GetUpvalue(std::size_t index) const
        { return &upvalues_[index]; }

        // Get module name
        String * GetModule() const
        { return module_; }

        // Get line of function define
        int GetLine() const
        { return line_; }

    private:
        // For debug
        struct LocalVarInfo
        {
            // Local variable name
            String *name_;
            // Register id in function
            int register_id_;
            // Begin instruction index of variable
            int begin_pc_;
            // The past-the-end instruction index
            int end_pc_;

            LocalVarInfo(String *name, int register_id,
                         int begin_pc, int end_pc)
                : name_(name), register_id_(register_id),
                  begin_pc_(begin_pc), end_pc_(end_pc) { }
        };

        // function instruction opcodes
        std::vector<Instruction> opcodes_;
        // opcodes' line number
        std::vector<int> opcode_lines_;
        // const values in function
        std::vector<Value> const_values_;
        // debug info
        std::vector<LocalVarInfo> local_vars_;
        // child functions
        std::vector<Function *> child_funcs_;
        // upvalues
        std::vector<UpvalueInfo> upvalues_;
        // function define module name
        String *module_;
        // function define line at module
        int line_;
        // count of args
        int args_;
        // has '...' param or not
        bool is_vararg_;
        // superior function pointer
        Function *superior_;
    };

    // All runtime function are closures, this class object pointer to a
    // prototype Function object and its upvalues.
    class Closure : public GCObject
    {
    public:
        Closure();

        virtual void Accept(GCObjectVisitor *v);

        // Get and set closure prototype Function
        Function * GetPrototype() const;
        void SetPrototype(Function *prototype);

        // Add upvalue
        void AddUpvalue(Upvalue *upvalue)
        { upvalues_.push_back(upvalue); }

        // Get upvalue by index
        Upvalue * GetUpvalue(std::size_t index) const
        { return upvalues_[index]; }

    private:
        // prototype Function
        Function *prototype_;
        // upvalues
        std::vector<Upvalue *> upvalues_;
    };
} // namespace luna

#endif // FUNCTION_H
