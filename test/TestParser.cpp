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
    ParserWrapper parser("a = -123 ^ 2 ^ -2 * 1 / 2 % 2 * 2 ^ 10 + 10 - 5 .. 'str' == 'str' and true or false");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser2)
{
    ParserWrapper parser("function f(a, b, c, ...) f(a, b, c); t.a, t.b, t.c = a, b, c return a, b, c; end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser3)
{
    ParserWrapper parser("t = {['str'] = 1 ^ 2, abc = 'str' .. 2, id, 1 + 2;}");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser4)
{
    ParserWrapper parser("a = (1 + 2) * 3 / 4");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser5)
{
    ParserWrapper parser("local name");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser6)
{
    ParserWrapper parser("table[index] = 1");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser7)
{
    ParserWrapper parser("t.a.b.c = 1");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser8)
{
    ParserWrapper parser("f(a, b, c)");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser9)
{
    ParserWrapper parser("f:m()");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser10)
{
    ParserWrapper parser("f{1, 2, 3}");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser11)
{
    ParserWrapper parser("f:m'str'");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser12)
{
    ParserWrapper parser("f(1, 2, 3):m{1, 2, 3}.m[123].m = 1");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser13)
{
    ParserWrapper parser("function f() do end end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser14)
{
    ParserWrapper parser("function f() while true do return end end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser15)
{
    ParserWrapper parser("function f() repeat return until (1 + 1) end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser16)
{
    ParserWrapper parser("function f() local function f() end local a, b, c = 1, 2 end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser17)
{
    ParserWrapper parser("function f() function a.b.c:d() return end end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser18)
{
    ParserWrapper parser("function f() for a = 1, 2, 3 do end for a, b in pairs(t) do end end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}

TEST_CASE(parser19)
{
    ParserWrapper parser("function f() if 1 + 1 then elseif not true then else end end");

    EXPECT_TRUE(parser.Parse());
    EXPECT_TRUE(parser.IsEOF());
}
