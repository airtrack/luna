#ifndef NUMBER_H
#define NUMBER_H

#include "Value.h"

namespace lua
{
    class Number : public Value
    {
    public:
        explicit Number(double v)
            : value_(v)
        {
        }

        virtual int Type() const
        {
            return TYPE_NUMBER;
        }

        virtual std::string Name() const
        {
            return "number";
        }

        bool IsInteger() const
        {
            return false;
        }

        int ConvertToInteger() const
        {
            return 0;
        }

    private:
        double value_;
    };
} // namespace lua

#endif // NUMBER_H
