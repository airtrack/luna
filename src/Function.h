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
        Function();

        virtual void Accept(GCObjectVisitor *v);

        // Get function instructions and size
        const Instruction * GetOpCodes() const;
        std::size_t OpCodeSize() const;

        // Add instruction, 'line' is line number of the instruction 'i'
        void AddInstruction(Instruction i, int line);

        // Set and get this function has vararg
        void SetHasVararg();
        bool HasVararg() const;

        // Set and get fixed arg count
        void SetFixedArgCount(int count);
        int FixedArgCount() const;

        // Set module and function define start line
        void SetBaseInfo(String *module, int line);

        // Set superior function
        void SetSuperior(Function *superior);

        // Add const number and return index of the const value
        int AddConstNumber(double num);

        // Add const String and return index of the const value
        int AddConstString(String *str);

        // Add const Value and return index of the const value
        int AddConstValue(const Value &v);

        // Get const Value by index
        Value * GetConstValue(int i);

        // Get instruction line by instruction index
        int GetInstructionLine(int i) const;

    private:
        static const int kMaxRegisterCount = 250;

        // function instruction opcodes
        std::vector<Instruction> opcodes_;
        // opcodes' line number
        std::vector<int> opcode_lines_;
        // const values in function
        std::vector<Value> const_values_;
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

        // Get upvalue by index
        Value * GetUpvalue(std::size_t index) const;

    private:
        // prototype Function
        Function *prototype_;
        // upvalues
        std::vector<Upvalue *> upvalues_;
    };
} // namespace luna

#endif // FUNCTION_H
