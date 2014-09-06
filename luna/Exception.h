#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "Token.h"
#include "Value.h"
#include "String.h"
#include <string>
#include <sstream>
#include <utility>

namespace luna
{
    // Base exception for luna, all exceptions throwed by luna
    // are derived from this class
    class Exception
    {
    public:
        const std::string& What() const { return what_; }

    protected:
        // Helper functions for format string of exception
        void SetWhat(std::ostringstream &) { }

        template<typename Arg, typename... Args>
        void SetWhat(std::ostringstream &oss, Arg&& arg, Args&&... args)
        {
            oss << std::forward<Arg>(arg);
            SetWhat(oss, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void SetWhat(Args&&... args)
        {
            std::ostringstream oss;
            SetWhat(oss, std::forward<Args>(args)...);
            what_ = oss.str();
        }

    private:
        std::string what_;
    };

    // Module file open failed, this exception will be throwed
    class OpenFileFail : public Exception
    {
    public:
        explicit OpenFileFail(const std::string &file)
        {
            SetWhat(file);
        }
    };

    // For lexer report error of token
    class LexException : public Exception
    {
    public:
        template<typename... Args>
        LexException(const char *module, int line, int column, Args&&... args)
        {
            SetWhat(module, ':', line, ':', column, ' ',
                    std::forward<Args>(args)...);
        }
    };

    // For parser report grammer error
    class ParseException : public Exception
    {
    public:
        ParseException(const char *str, const TokenDetail &t)
        {
            SetWhat(t.module_->GetCStr(), ':', t.line_, ':', t.column_, " '",
                    GetTokenStr(t), "' ", str);
        }
    };

    // For semantic analysor report semantic error
    class SemanticException : public Exception
    {
    public:
        SemanticException(const char *str, const TokenDetail &t)
        {
            SetWhat(t.module_->GetCStr(), ':', t.line_, ':', t.column_, " '",
                    GetTokenStr(t), "' ", str);
        }
    };

    // For code generator report error
    class CodeGenerateException : public Exception
    {
    public:
        template<typename... Args>
        CodeGenerateException(const char *module, int line, Args&&... args)
        {
            SetWhat(module, ':', line, ' ', std::forward<Args>(args)...);
        }
    };

    // Report error of call c function
    class CallCFuncException : public Exception
    {
    public:
        explicit CallCFuncException(const char *what)
        {
            SetWhat(what);
        }
    };

    // For VM report runtime error
    class RuntimeException : public Exception
    {
    public:
        RuntimeException(const char *module, int line, const char *desc)
        {
            SetWhat(module, ':', line, ' ', desc);
        }

        RuntimeException(const char *module, int line,
                         const Value *v, const char *v_name,
                         const char *expect_type)
        {
            SetWhat(module, ':', line, ' ', v_name, " is a ",
                    v->TypeName(), " value, expect a ", expect_type, " value");
        }

        RuntimeException(const char *module, int line,
                         const Value *v, const char *v_name,
                         const char *v_scope, const char *op)
        {
            SetWhat(module, ':', line, " attempt to ", op, ' ',
                    v_scope, " '", v_name, "' (a ", v->TypeName(), " value)");
        }

        RuntimeException(const char *module, int line,
                         const Value *v1, const Value *v2,
                         const char *op)
        {
            SetWhat(module, ':', line, " attempt to ", op, ' ',
                    v1->TypeName(), " with ", v2->TypeName());
        }
    };
} // namespace luna

#endif // EXCEPTION_H
