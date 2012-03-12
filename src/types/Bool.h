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

        virtual std::size_t GetHash() const
        {
            return value_ ? 1 : 0;
        }

        virtual bool IsEqual(const Value *other) const
        {
            if (this == other)
                return true;
            if (Type() != other->Type())
                return false;
            return value_ == static_cast<const Bool *>(other)->value_;
        }

        bool Get() const
        {
            return value_;
        }

    private:
        bool value_;
    };
} // namespace lua

#endif // BOOL_H
