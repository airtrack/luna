#ifndef NIL_H
#define NIL_H

#include "Value.h"

namespace lua
{
    class Nil : public Value
    {
    public:
        virtual int Type() const;
        virtual std::string Name() const
        {
            return "nil";
        }
    };
} // namespace lua

#endif // NIL_H
