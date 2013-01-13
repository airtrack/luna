#include "Parser.h"
#include "Lex.h"
#include "State.h"

namespace
{
    using namespace luna;

    class ParserImpl
    {
    public:
        ParserImpl(State *state, Lexer *lexer)
            : state_(state), lexer_(lexer)
        {
        }

        std::unique_ptr<SyntaxTree> Parse()
        {
            return std::unique_ptr<SyntaxTree>();
        }

    private:
        State *state_;
        Lexer *lexer_;
    };
} // namespace

namespace luna
{
    Parser::Parser(State *state)
        : state_(state)
    {
    }

    std::unique_ptr<SyntaxTree> Parser::Parse(Lexer *lexer)
    {
        ParserImpl impl(state_, lexer);
        return impl.Parse();
    }
} // namespace luna
