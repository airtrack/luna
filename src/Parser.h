#ifndef PARSER_H
#define PARSER_H

#include "Source.h"
#include "Lexer.h"
#include "LexTable.h"
#include "ParseTreeNode.h"
#include <memory>

namespace lua
{
    class Parser
    {
    public:
        explicit Parser(Source *source);

        std::unique_ptr<ParseTreeNode> Parse();

    private:
        LexTable lex_table_;
        Lexer lexer_;
    };
} // namespace lua

#endif // PARSER_H
