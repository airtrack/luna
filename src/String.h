#ifndef STRING_H
#define STRING_H

#include <string>

namespace luna
{
    class String
    {
    public:
        explicit String(const std::string &str) : str_(str) { }

    private:
        std::string str_;
    };
} // namespace luna

#endif // STRING_H
