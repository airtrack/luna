#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "Token.h"
#include "Value.h"
#include <stdio.h>
#include <string>
#include <utility>

#ifdef _MSC_VER
#define snprintf(buffer, size, fmt, ...)	\
    _snprintf_s(buffer, size, _TRUNCATE, fmt, __VA_ARGS__)
#endif // _MSC_VER

namespace luna
{
    class Exception
    {
    public:
        std::string What() const { return what_; }

    protected:
        std::string what_;
    };

    class OpenFileFail : public Exception
    {
    public:
        explicit OpenFileFail(const std::string &file)
        {
            what_ = file;
        }
    };

    class LexException : public Exception
    {
    public:
        LexException(int line, int column, const char *str)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), "%d:%d %s", line, column, str);
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
        ParseException(const char *str, const TokenDetail &t)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), "%d:%d '%s' %s", t.line_, t.column_,
                     GetTokenStr(t).c_str(), str);
            what_ = buffer;
        }
    };

    class SemanticException : public Exception
    {
    public:
        SemanticException(const char *str, const TokenDetail &t)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), "%d:%d '%s' %s", t.line_, t.column_,
                     GetTokenStr(t).c_str(), str);
            what_ = buffer;
        }
    };

    class CodeGenerateException : public Exception
    {
    public:
        template<typename... Args>
        CodeGenerateException(const char *format, Args&&... args)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
            what_ = buffer;
        }
    };

    class CallCFuncException : public Exception
    {
    public:
        explicit CallCFuncException(const char *what)
        {
            what_ = what;
        }
    };

    class RuntimeException : public Exception
    {
    public:
        RuntimeException(const Value *v, const char *v_name,
                         const char *v_scope, const char *op,
                         int line, const char *module)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), "%s:%d: attempt to %s %s '%s' (a %s value)",
                     module, line, op, v_scope, v_name, v->TypeName());
            what_ = buffer;
        }

        RuntimeException(const Value *v1, const Value *v2, const char *op,
                         int line, const char *module)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), "%s:%d: attempt to %s %s with %s",
                     module, line, op, v1->TypeName(), v2->TypeName());
            what_ = buffer;
        }

        RuntimeException(const Value *v, const char *v_name, const char *expect_type,
                         int line, const char *module)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), "%s:%d: %s is a %s value, expect a %s value",
                     module, line, v_name, v->TypeName(), expect_type);
            what_ = buffer;
        }

        RuntimeException(const char *desc, int line, const char *module)
        {
            char buffer[128] = { 0 };
            snprintf(buffer, sizeof(buffer), "%s:%d: %s",
                     module, line, desc);
            what_ = buffer;
        }
    };
} // namespace luna

#endif // EXCEPTION_H
