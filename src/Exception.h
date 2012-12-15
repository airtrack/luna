#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

namespace luna
{
    class Exception
    {
    public:
        explicit Exception(const std::string &what) : what_(what) { }

        std::string What() const { return what_; }

    private:
        std::string what_;
    };

    class LexException : public Exception
    {
    public:
        explicit LexException(const std::string &what) : Exception(what) { }
    };
} // namespace luna

#endif // EXCEPTION_H
