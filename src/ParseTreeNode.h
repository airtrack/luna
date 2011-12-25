#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

namespace lua
{
    class Lexer;

    class ParseTreeNode
    {
    public:
        virtual bool ParseNode(Lexer *lexer) = 0;
        virtual ~ParseTreeNode() {}
    };
} // namespace lua

#endif // PARSE_TREE_NODE_H
