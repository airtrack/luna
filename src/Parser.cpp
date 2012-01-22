#include "Parser.h"
#include "Statement.h"

namespace lua
{
    Parser::Parser(Source *source)
        : lexer_(source, &lex_table_)
    {
    }

    ParseTreeNodePtr Parser::Parse()
    {
        return ParseChunkStatement(&lexer_);
    }
} // namespace lua
