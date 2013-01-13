#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdio.h>
#include <string>
#include <utility>

namespace luna
{
    class Exception
    {
    public:
        std::string What() const { return what_; }

    protected:
        std::string what_;
    };

    class LexException : public Exception
    {
    public:
        LexException(int line, int column, const char *str)
        {
            char buffer[128] = { 0 };
            int len = snprintf(buffer, sizeof(buffer), "%d:%d ", line, column);
            snprintf(buffer + len, sizeof(buffer) - len, "%s", str);
            what_ = buffer;
        }

        template<typename... Args>
        LexException(int line, int column, const char *format, Args&&... args)
        {
            char buffer[128] = { 0 };
            int len = snprintf(buffer, sizeof(buffer), "%d:%d ", line, column);
            snprintf(buffer + len, sizeof(buffer) - len, format, std::forward<Args>(args)...);
            what_ = buffer;
        }
    };

    class ParseException : public Exception
    {
    public:
    };
} // namespace luna

#endif // EXCEPTION_H
