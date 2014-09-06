#ifndef LEX_H
#define LEX_H

#include "Token.h"
#include <string>
#include <functional>

namespace luna
{
    class String;
    class State;

    class Lexer
    {
    public:
        typedef std::function<int ()> CharInStream;

        Lexer(State *state, String *module, CharInStream in);

        Lexer(const Lexer&) = delete;
        void operator = (const Lexer&) = delete;

        // Get next token, 'detail' store next token detail information,
        // return value is next token type.
        int GetToken(TokenDetail *detail);

        // Get current lex module name.
        String * GetLexModule() const
        {
            return module_;
        }

    private:
        int Next()
        {
            auto c = in_stream_();
            if (c != EOF) ++column_;
            return c;
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
        String *module_;
        CharInStream in_stream_;

        int current_;
        int line_;
        int column_;

        std::string token_buffer_;
    };
} // namespace luna

#endif // LEX_H
