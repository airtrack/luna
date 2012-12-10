#ifndef LEX_H
#define LEX_H

#include <functional>

namespace luna
{
    class String;

    enum Token
    {
        Token_Id = 128, Token_KeyWord, Token_String, Token_Number,
        Token_Equal, Token_NotEqual, Token_LessEqual, Token_BigEqual,
        Token_Concat, Token_VarArg,
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
        int column_;                // token column number in the 'line_'
    };

    class Lexer
    {
    public:
        typedef std::function<int ()> CharInStream;

        explicit Lexer(CharInStream in);

        Lexer(const Lexer&) = delete;
        void operator = (const Lexer&) = delete;

        int GetToken(TokenDetail *detail);

    private:
        CharInStream in_stream_;
    };
} // namespace luna

#endif // LEX_H
