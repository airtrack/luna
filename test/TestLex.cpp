#include "UnitTest.h"
#include "Lex.h"
#include "State.h"
#include "String.h"
#include "TextInStream.h"
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
    LexerWrapper lexer("");
    EXPECT_TRUE(lexer.GetToken() == luna::Token_EOF);
}
