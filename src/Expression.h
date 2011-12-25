#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ParseTreeNode.h"

namespace lua
{
    class Expression : public ParseTreeNode
    {
    public:
    };

    class BasicExpression : public Expression
    {
    public:
        virtual bool ParseNode(Lexer *lexer);
    };
} // namespace lua

#endif // EXPRESSION_H
