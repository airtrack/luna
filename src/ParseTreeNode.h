#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <memory>

namespace lua
{
	class Lexer;

    class ParseTreeNode
    {
    public:
        virtual ~ParseTreeNode() {}
    };

    class Statement : public ParseTreeNode
    {
    public:
    };

    class Expression : public ParseTreeNode
    {
    public:
    };

    typedef std::unique_ptr<ParseTreeNode> ParseTreeNodePtr;
    typedef std::unique_ptr<Statement> StatementPtr;
    typedef std::unique_ptr<Expression> ExpressionPtr;

#define THROW_PARSER_ERROR(desc) \
    Error::ThrowError(           \
        lexer->GetLineNumber(),  \
        lexer->GetColumnNumber(),\
        desc)
} // namespace lua

#endif // PARSE_TREE_NODE_H
