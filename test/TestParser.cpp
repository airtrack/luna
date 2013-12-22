#include "UnitTest.h"
#include "TestCommon.h"

namespace
{
    ParserWrapper g_parser;
    std::unique_ptr<luna::SyntaxTree> Parse(const std::string &s)
    {
        g_parser.SetInput(s);
        return g_parser.Parse();
    }

    bool IsEOF()
    {
        return g_parser.IsEOF();
    }
} // namespace

TEST_CASE(parser1)
{
    EXPECT_TRUE(Parse("a = -123 ^ 2 ^ -2 * 1 / 2 % 2 * 2 ^ 10 + 10 - 5 .. 'str' == 'str' and true or false"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser2)
{
    EXPECT_TRUE(Parse("function f(a, b, c, ...) f(a, b, c); t.a, t.b, t.c = a, b, c return a, b, c; end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser3)
{
    EXPECT_TRUE(Parse("t = {['str'] = 1 ^ 2, abc = 'str' .. 2, id, 1 + 2;}"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser4)
{
    EXPECT_TRUE(Parse("a = (1 + 2) * 3 / 4"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser5)
{
    EXPECT_TRUE(Parse("local name"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser6)
{
    EXPECT_TRUE(Parse("table[index] = 1"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser7)
{
    EXPECT_TRUE(Parse("t.a.b.c = 1"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser8)
{
    EXPECT_TRUE(Parse("f(a, b, c)"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser9)
{
    EXPECT_TRUE(Parse("f:m()"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser10)
{
    EXPECT_TRUE(Parse("f{1, 2, 3}"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser11)
{
    EXPECT_TRUE(Parse("f:m'str'"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser12)
{
    EXPECT_TRUE(Parse("f(1, 2, 3):m{1, 2, 3}.m[123].m = 1"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser13)
{
    EXPECT_TRUE(Parse("function f() do end end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser14)
{
    EXPECT_TRUE(Parse("function f() while true do return end end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser15)
{
    EXPECT_TRUE(Parse("function f() repeat return until (1 + 1) end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser16)
{
    EXPECT_TRUE(Parse("function f() local function f() end local a, b, c = 1, 2 end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser17)
{
    EXPECT_TRUE(Parse("function f() function a.b.c:d() return end end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser18)
{
    EXPECT_TRUE(Parse("function f() for a = 1, 2, 3 do end for a, b in pairs(t) do end end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser19)
{
    EXPECT_TRUE(Parse("function f() if 1 + 1 then elseif not true then else end end"));
    EXPECT_TRUE(IsEOF());
}

TEST_CASE(parser20)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("return return");
                     });
}

TEST_CASE(parser21)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("t = {} t.a = ");
                     });
}

TEST_CASE(parser22)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("f().m():m().m");
                     });
}

TEST_CASE(parser23)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("do end end");
                     });
}

TEST_CASE(parser24)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("while true, false do end");
                     });
}

TEST_CASE(parser25)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("repeat until;");
                     });
}

TEST_CASE(parser26)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("if true false then end");
                     });
}

TEST_CASE(parser27)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("if true then elseif false then else");
                     });
}

TEST_CASE(parser28)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("for a.b = 1, 2 do end");
                     });
}

TEST_CASE(parser29)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("for a = 1, 2, 3, 4 do end");
                     });
}

TEST_CASE(parser30)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("for a.b in pairs(t) do end");
                     });
}

TEST_CASE(parser31)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("function a.b.c:m.c() end");
                     });
}

TEST_CASE(parser32)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("local function a.b() end");
                     });
}

TEST_CASE(parser33)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("local function a(m, ..., n) end");
                     });
}

TEST_CASE(parser34)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("local a = 1, 2,");
                     });
}

TEST_CASE(parser35)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("t = {a.b = 1}");
                     });
}

TEST_CASE(parser36)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("local a end");
                     });
}

TEST_CASE(parser37)
{
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         Parse("f 1");
                     });
}
