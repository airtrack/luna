#ifndef STRING_H
#define STRING_H

#include "Value.h"

namespace lua
{
    class String : public Value
    {
    public:
        explicit String(const std::string& v)
            : value_(v)
        {
        }

        virtual int Type() const
        {
            return TYPE_STRING;
        }

        virtual std::string Name() const
        {
            return "string";
        }

        virtual std::size_t GetHash() const;
        virtual bool IsEqual(const Value *other) const;
        virtual void Mark();

        std::string Get()
        {
            return value_;
        }

        const std::string& Get() const
        {
            return value_;
        }

    private:
        std::string value_;
    };
} // namespace lua

#endif // STRING_H
