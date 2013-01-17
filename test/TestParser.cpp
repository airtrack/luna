#include "UnitTest.h"
#include "Lex.h"
#include "Parser.h"
#include "State.h"
#include "String.h"
#include "TextInStream.h"
#include <functional>

namespace
{
    class ParserWrapper
    {
    public:
        explicit ParserWrapper(const std::string &str)
            : iss_(str), state_(), name_("parser"),
              lexer_(&state_, &name_, std::bind(&io::text::InStringStream::GetChar, &iss_)), parser_(&state_)
        {
        }

        bool IsEOF()
        {
            luna::TokenDetail detail;
            return lexer_.GetToken(&detail) == luna::Token_EOF;
        }

        std::unique_ptr<luna::SyntaxTree> Parse()
        {
            return parser_.Parse(&lexer_);
        }

    private:
        io::text::InStringStream iss_;
        luna::State state_;
        luna::String name_;
        luna::Lexer lexer_;
        luna::Parser parser_;
    };
} // namespace

TEST_CASE(parser1)
{
    ParserWrapper parser("-123 ^ 2 ^ -2 * 1 / 2 % 2 * 2 ^ 10 + 10 - 5 .. 'str' == 'str' and true or false");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser2)
{
    ParserWrapper parser("function(a, b, c, ...) end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}
