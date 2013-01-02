#include "UnitTest.h"
#include "Lex.h"
#include "Parser.h"
#include "State.h"
#include "String.h"
#include "TextInStream.h"
#include <functional>

TEST_CASE(parser1)
{
    io::text::InStringStream iss("-123 ^ 2 * 1 / 2 % 2 * 2 ^ 10 + 10 - 5 .. 'str' == 'str' and true or false");
    luna::State state;
    luna::String name("parser");
    luna::Lexer lexer(&state, &name, std::bind(&io::text::InStringStream::GetChar, &iss));
    luna::Parser parser;
    luna::TokenDetail detail;
}
