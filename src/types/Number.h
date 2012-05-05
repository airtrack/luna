#ifndef NUMBER_H
#define NUMBER_H

#include "Value.h"
#include <math.h>

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

        virtual std::size_t GetHash() const;
        virtual bool IsEqual(const Value *other) const;
        virtual void Mark();

        double Get() const
        {
            return value_;
        }

        void Set(double v)
        {
            value_ = v;
        }

        bool IsInteger() const
        {
            return floor(value_) == value_;
        }

        int GetInteger() const
        {
            return static_cast<int>(floor(value_));
        }

    private:
        double value_;
    };
} // namespace lua

#endif // NUMBER_H
