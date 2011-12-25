#include "Parser.h"
#include "Statement.h"

namespace lua
{
    Parser::Parser(Source *source)
        : lexer_(source, &lex_table_)
    {
    }

    std::unique_ptr<ParseTreeNode> Parser::Parse()
    {
        std::unique_ptr<ParseTreeNode> root(new ChunkStatement);
        root->ParseNode(&lexer_);
        return root;
    }
} // namespace lua
