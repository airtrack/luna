#ifndef STRING_H
#define STRING_H

#include "GCObject.h"
#include <string>

namespace luna
{
    class String : public GCObject
    {
    public:
        explicit String(const std::string &str) : str_(str) { }

        const std::string& GetStdString() const
        { return str_; }

    private:
        std::string str_;
    };
} // namespace luna

#endif // STRING_H
