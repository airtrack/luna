#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace luna
{
    class String;

    enum Token
    {
        Token_And = 256, Token_Do, Token_Else, Token_Elseif, Token_End,
        Token_False, Token_For, Token_Function, Token_If, Token_In,
        Token_Local, Token_Nil, Token_Not, Token_Or, Token_Repeat,
        Token_Return, Token_Then, Token_True, Token_Until, Token_While,
        Token_Id, Token_String, Token_Number,
        Token_Equal, Token_NotEqual, Token_LessEqual, Token_BigEqual,
        Token_Concat, Token_VarArg, Token_EOF,
    };

    struct TokenDetail
    {
        union
        {
            double number_;         // number for Token_Number
            String *str_;           // string for Token_Id, Token_KeyWord and Token_String
        };

        String *module_;            // module name of this token belongs to
        int line_;                  // token line number in module
        int column_;                // token column number at 'line_'
        int token_;                 // token value

        TokenDetail() : str_(nullptr), module_(nullptr), line_(0), column_(0), token_(Token_EOF) { }
    };

    std::string GetTokenStr(const TokenDetail &t);
} // namespace luna

#endif // TOKEN_H
