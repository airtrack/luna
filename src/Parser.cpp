#include "Parser.h"
#include "Statement.h"

namespace lua
{
    Parser::Parser(Source *source, State *state)
        : lexer_(source, &lex_table_, state)
    {
    }

    ParseTreeNodePtr Parser::Parse()
    {
        return ParseChunkStatement(&lexer_);
    }
} // namespace lua
