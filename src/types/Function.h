#ifndef FUNCTION_H
#define FUNCTION_H

#include "Value.h"
#include "../Instruction.h"

namespace lua
{
    class Function : public Value, public InstructionSet
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
    };
} // namespace lua

#endif // FUNCTION_H
