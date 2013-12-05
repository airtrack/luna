#include "UnitTest.h"
#include "TestCommon.h"

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

TEST_CASE(parser20)
{
    ParserWrapper parser("return return");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser21)
{
    ParserWrapper parser("t = {} t.a = ");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser22)
{
    ParserWrapper parser("f().m():m().m");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser23)
{
    ParserWrapper parser("do end end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser24)
{
    ParserWrapper parser("while true, false do end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser25)
{
    ParserWrapper parser("repeat until;");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser26)
{
    ParserWrapper parser("if true false then end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser27)
{
    ParserWrapper parser("if true then elseif false then else");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser28)
{
    ParserWrapper parser("for a.b = 1, 2 do end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser29)
{
    ParserWrapper parser("for a = 1, 2, 3, 4 do end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser30)
{
    ParserWrapper parser("for a.b in pairs(t) do end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser31)
{
    ParserWrapper parser("function a.b.c:m.c() end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser32)
{
    ParserWrapper parser("local function a.b() end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser33)
{
    ParserWrapper parser("local function a(m, ..., n) end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser34)
{
    ParserWrapper parser("local a = 1, 2,");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser35)
{
    ParserWrapper parser("t = {a.b = 1}");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser36)
{
    ParserWrapper parser("local a end");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}

TEST_CASE(parser37)
{
    ParserWrapper parser("f 1");
    EXPECT_EXCEPTION(luna::ParseException,
                     {
                         parser.Parse();
                     });
}
