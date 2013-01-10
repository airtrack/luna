#ifndef STRING_H
#define STRING_H

#include "GCObject.h"
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

        const std::string& GetStdString() const
        { return str_; }

    private:
        void SetValue(const std::string &str)
        { str_ = str; }

        std::string str_;
    };
} // namespace luna

#endif // STRING_H
