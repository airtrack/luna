#ifndef FUNCTION_H
#define FUNCTION_H

#include "Value.h"
#include "../Instruction.h"
#include <vector>

namespace lua
{
    class Function : public Value
    {
    public:
        virtual int Type() const
        {
            return TYPE_FUNCTION;
        }

        virtual std::string Name() const
        {
            return "function";
        }

        virtual std::size_t GetHash() const;
        virtual bool IsEqual(const Value *other) const;

        // Add new instruction to function, and return the instruction pointer.
        Instruction * NewInstruction();

        // Get instructions base pointer.
        Instruction * GetInstructions();

        // Get count of instructions.
        std::size_t GetInstructionCount();

    private:
        std::vector<Instruction> instructions_;
    };
} // namespace lua

#endif // FUNCTION_H
