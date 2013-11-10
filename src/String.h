#ifndef STRING_H
#define STRING_H

#include "GC.h"
#include <string>

namespace luna
{
    class StringPool;

    class String : public GCObject
    {
        friend class StringPool;
    public:
        String() { }
        explicit String(const std::string &str) : str_(str) { }

        virtual void Accept(GCObjectVisitor *v)
        {
            v->Visit(this);
        }

        const std::string& GetStdString() const
        { return str_; }

        void SetValue(const std::string &str)
        { str_ = str; }

    private:
        std::string str_;
    };
} // namespace luna

#endif // STRING_H
