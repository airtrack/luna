#ifndef NIL_H
#define NIL_H

#include "Value.h"

namespace lua
{
    class Nil : public Value
    {
    public:
        virtual int Type() const
        {
            return TYPE_NIL;
        }

        virtual std::string Name() const
        {
            return "nil";
        }

        virtual std::size_t GetHash() const
        {
            return 0;
        }

        virtual bool IsEqual(const Value *other) const
        {
            if (this == other)
                return true;
            return Type() == other->Type();
        }

        virtual void Mark()
        {
            MarkSelf();
        }
    };
} // namespace lua

#endif // NIL_H
