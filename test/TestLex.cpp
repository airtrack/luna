#include "UnitTest.h"
#include "../src/Lex.h"
#include "../src/State.h"
#include "../src/String.h"
#include "../src/TextInStream.h"
#include "../src/Exception.h"
#include <functional>

namespace
{
    class LexerWrapper
    {
    public:
        explicit LexerWrapper(const std::string &str)
            : iss_(str),
              state_(),
              name_("lex"),
              lexer_(&state_, &name_, std::bind(&io::text::InStringStream::GetChar, &iss_))
        {
        }

        int GetToken()
        {
            luna::TokenDetail token;
            return lexer_.GetToken(&token);
        }

    private:
        io::text::InStringStream iss_;
        luna::State state_;
        luna::String name_;
        luna::Lexer lexer_;
    };
} // namespace

TEST_CASE(lex1)
{
    LexerWrapper lexer("\r\n\t\v\f ");
    EXPECT_TRUE(lexer.GetToken() == luna::Token_EOF);
}

TEST_CASE(lex2)
{
    LexerWrapper lexer("-- this is comment\n"
                       "--[[this is long\n comment]]"
                       "--[[this is long\n comment too--]]"
                       "--[[incomplete comment]");

    EXPECT_EXCEPTION(luna::LexException, {
        lexer.GetToken();
    });
}

TEST_CASE(lex3)
{
    LexerWrapper lexer("[==[long\nlong\nstring]==]'string'\"string\""
                       "[=[incomplete string]=");
    for (int i = 0; i < 3; ++i)
        EXPECT_TRUE(lexer.GetToken() == luna::Token_String);

    EXPECT_EXCEPTION(luna::LexException, {
        lexer.GetToken();
    });
}

TEST_CASE(lex4)
{
    LexerWrapper lexer("3 3.0 3.1416 314.16e-2 0.31416E1 0xff 0x0.1E 0xA23p-4 0X1.921FB54442D18P+1"
                       " 0x");
    for (int i = 0; i < 9; ++i)
        EXPECT_TRUE(lexer.GetToken() == luna::Token_Number);

    EXPECT_EXCEPTION(luna::LexException, {
        lexer.GetToken();
    });
}

TEST_CASE(lex5)
{
    LexerWrapper lexer("+ - * / % ^ # == ~= <= >= < > = ( ) { } [ ] ; : , . .. ...");
    EXPECT_TRUE(lexer.GetToken() == '+');
    EXPECT_TRUE(lexer.GetToken() == '-');
    EXPECT_TRUE(lexer.GetToken() == '*');
    EXPECT_TRUE(lexer.GetToken() == '/');
    EXPECT_TRUE(lexer.GetToken() == '%');
    EXPECT_TRUE(lexer.GetToken() == '^');
    EXPECT_TRUE(lexer.GetToken() == '#');
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Equal);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_NotEqual);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_LessEqual);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_GreaterEqual);
    EXPECT_TRUE(lexer.GetToken() == '<');
    EXPECT_TRUE(lexer.GetToken() == '>');
    EXPECT_TRUE(lexer.GetToken() == '=');
    EXPECT_TRUE(lexer.GetToken() == '(');
    EXPECT_TRUE(lexer.GetToken() == ')');
    EXPECT_TRUE(lexer.GetToken() == '{');
    EXPECT_TRUE(lexer.GetToken() == '}');
    EXPECT_TRUE(lexer.GetToken() == '[');
    EXPECT_TRUE(lexer.GetToken() == ']');
    EXPECT_TRUE(lexer.GetToken() == ';');
    EXPECT_TRUE(lexer.GetToken() == ':');
    EXPECT_TRUE(lexer.GetToken() == ',');
    EXPECT_TRUE(lexer.GetToken() == '.');
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Concat);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_VarArg);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_EOF);
}

TEST_CASE(lex6)
{
    LexerWrapper lexer("and do else elseif end false for function if in local "
                       "nil not or repeat return then true until while");
    EXPECT_TRUE(lexer.GetToken() == luna::Token_And);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Do);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Else);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Elseif);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_End);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_False);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_For);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Function);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_If);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_In);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Local);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Nil);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Not);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Or);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Repeat);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Return);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Then);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_True);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_Until);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_While);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_EOF);
}

TEST_CASE(lex7)
{
    LexerWrapper lexer("_ __ ___ _1 _a _a1 a1 a_ a_1 name");
    for (int i = 0; i < 10; ++i)
        EXPECT_TRUE(lexer.GetToken() == luna::Token_Id);
    EXPECT_TRUE(lexer.GetToken() == luna::Token_EOF);
}
