#ifndef LEX_H
#define LEX_H

#include <string>
#include <functional>

namespace luna
{
    class String;
    class State;

    enum Token
    {
        Token_Id = 128, Token_KeyWord, Token_String, Token_Number,
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
        int column_;                // token column number in the 'line_'
    };

    class Lexer
    {
    public:
        typedef std::function<int ()> CharInStream;

        Lexer(State *state, CharInStream in);

        Lexer(const Lexer&) = delete;
        void operator = (const Lexer&) = delete;

        int GetToken(TokenDetail *detail);

    private:
        int Next()
        {
            ++column_;
            return in_stream_();
        }

        void LexNewLine();
        void LexComment();
        void LexMultiLineComment();
        void LexSingleLineComment();

        int LexNumber(TokenDetail *detail);
        int LexNumberX(TokenDetail *detail, bool integer_part,
                       std::function<bool (int)> is_number_char,
                       std::function<bool (int)> is_exponent);
        int LexNumberXFractional(TokenDetail *detail,
                                 bool integer_part, bool point,
                                 std::function<bool (int)> is_number_char,
                                 std::function<bool (int)> is_exponent);

        int LexXEqual(TokenDetail *detail, int equal_token);

        int LexMultiLineString(TokenDetail *detail);
        int LexSingleLineString(TokenDetail *detail);
        void LexStringChar();

        int LexId(TokenDetail *detail);

        State *state_;
        CharInStream in_stream_;
        int current_;
        int line_;
        int column_;

        std::string token_buffer_;
    };
} // namespace luna

#endif // LEX_H
