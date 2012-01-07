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
        ParseTreeNodePtr root(new ChunkStatement);
        root->ParseNode(&lexer_);
        return root;
    }
} // namespace lua
