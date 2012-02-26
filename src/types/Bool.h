#ifndef BOOL_H
#define BOOL_H

#include "Value.h"

namespace lua
{
    class Bool : public Value
    {
    public:
        explicit Bool(bool v)
            : value_(v)
        {
        }

        virtual int Type() const
        {
            return TYPE_BOOL;
        }

        virtual std::string Name() const
        {
            return value_ ? "true" : "false";
        }

    private:
        bool value_;
    };
} // namespace lua

#endif // BOOL_H
