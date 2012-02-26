#ifndef FUNCTION_H
#define FUNCTION_H

#include "Value.h"

namespace lua
{
    class Function : public Value
    {
    public:
        virtual int Type() const;
        virtual std::string Name() const;
    };
} // namespace lua

#endif // FUNCTION_H
