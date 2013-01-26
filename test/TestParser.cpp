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

TEST_CASE(parser3)
{
    ParserWrapper parser("{['str'] = 1 ^ 2, abc = 'str' .. 2, id, 1 + 2;}");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser4)
{
    ParserWrapper parser("(1 + 2) * 3 / 4");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser5)
{
    ParserWrapper parser("name");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser6)
{
    ParserWrapper parser("table[index]");

    auto var = parser.Parse();
    EXPECT_TRUE(var);
    EXPECT_TRUE(dynamic_cast<luna::IndexAccessor *>(var.get()));
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser7)
{
    ParserWrapper parser("t.a.b.c");

    auto var = parser.Parse();
    EXPECT_TRUE(var);
    EXPECT_TRUE(dynamic_cast<luna::MemberAccessor *>(var.get()));
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser8)
{
    ParserWrapper parser("f(a, b, c)");

    auto func_call = parser.Parse();
    EXPECT_TRUE(func_call);
    EXPECT_TRUE(dynamic_cast<luna::NormalFuncCall *>(func_call.get()));
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser9)
{
    ParserWrapper parser("f:m()");

    auto func_call = parser.Parse();
    EXPECT_TRUE(func_call);
    EXPECT_TRUE(dynamic_cast<luna::MemberFuncCall *>(func_call.get()));
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser10)
{
    ParserWrapper parser("f{1, 2, 3}");

    auto func_call = parser.Parse();
    EXPECT_TRUE(func_call);
    EXPECT_TRUE(dynamic_cast<luna::NormalFuncCall *>(func_call.get()));
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser11)
{
    ParserWrapper parser("f:m'str'");

    auto func_call = parser.Parse();
    EXPECT_TRUE(func_call);
    EXPECT_TRUE(dynamic_cast<luna::MemberFuncCall *>(func_call.get()));
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser12)
{
    ParserWrapper parser("f(1, 2, 3):m{1, 2, 3}.m[123].m");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}
