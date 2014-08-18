#include "Token.h"
#include "String.h"
#include <stdio.h>

#ifdef _MSC_VER
#define snprintf(buffer, size, fmt, ...)	\
    _snprintf_s(buffer, size, _TRUNCATE, fmt, __VA_ARGS__)
#endif // _MSC_VER

namespace luna
{
    const char *token_str[] = {
        "and", "break", "do", "else", "elseif", "end",
        "false", "for", "function", "if", "in",
        "local", "nil", "not", "or", "repeat",
        "return", "then", "true", "until", "while",
        "<id>", "<string>", "<number>",
        "==", "~=", "<=", ">=", "..", "...", "<EOF>"
    };

    std::string GetTokenStr(const TokenDetail &t)
    {
        std::string str;

        int token = t.token_;
        if (token == Token_Number)
        {
            char number[32] = { 0 };
            snprintf(number, sizeof(number), "%g", t.number_);
            str = number;
        }
        else if (token == Token_Id || token == Token_String)
        {
            str = t.str_->GetStdString();
        }
        else if (token >= Token_And && token <= Token_EOF)
        {
            str = token_str[token - Token_And];
        }
        else
        {
            str.push_back(token);
        }

        return str;
    }
} // namespace luna
