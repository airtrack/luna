#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "Token.h"
#include "Visitor.h"
#include <memory>

namespace luna
{
    class SyntaxTree
    {
    public:
        virtual ~SyntaxTree() { }
        virtual void Accept(Visitor *v) = 0;
    };

    class Terminator : public SyntaxTree
    {
    public:
        TokenDetail token_;
    };

    class BinaryExpression : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> left_;
        std::unique_ptr<SyntaxTree> right_;
        TokenDetail op_token_;

        virtual void Accept(Visitor *v);
    };

    class UnaryExpression : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> exp_;
        TokenDetail op_token_;

        virtual void Accept(Visitor *v);
    };
} // namespace luna

#endif // SYNTAX_TREE_H
