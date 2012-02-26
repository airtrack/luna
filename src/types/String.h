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

        virtual int Type() const;

        virtual std::string Name() const
        {
            return "string";
        }

    private:
        std::string value_;
    };
} // namespace lua

#endif // STRING_H
