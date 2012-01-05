#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <memory>

namespace lua
{
    class Lexer;

    class ParseTreeNode
    {
    public:
        virtual bool ParseNode(Lexer *lexer) = 0;
        virtual ~ParseTreeNode() {}
    };

    typedef std::unique_ptr<ParseTreeNode> ParseTreeNodePtr;

#define THROW_PARSER_ERROR(desc) \
    ParserError::ThrowError(     \
        lexer->GetLineNumber(),  \
        lexer->GetColumnNumber(),\
        desc)

} // namespace lua

#endif // PARSE_TREE_NODE_H
