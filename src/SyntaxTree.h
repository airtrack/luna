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

        Terminator() { }
        explicit Terminator(const TokenDetail &token) : token_(token) { }

        virtual void Accept(Visitor *v);
    };

    class BinaryExpression : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> left_;
        std::unique_ptr<SyntaxTree> right_;
        TokenDetail op_token_;

        BinaryExpression() { }
        BinaryExpression(std::unique_ptr<SyntaxTree> left,
                         std::unique_ptr<SyntaxTree> right,
                         const TokenDetail &op)
            : left_(std::move(left)), right_(std::move(right)), op_token_(op)
        {
        }

        virtual void Accept(Visitor *v);
    };

    class UnaryExpression : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> exp_;
        TokenDetail op_token_;

        UnaryExpression() { }
        UnaryExpression(std::unique_ptr<SyntaxTree> exp,
                        const TokenDetail &op)
            : exp_(std::move(exp)), op_token_(op)
        {
        }

        virtual void Accept(Visitor *v);
    };

    class FunctionBody : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> param_list_;
        std::unique_ptr<SyntaxTree> block_;

        FunctionBody() { }
        FunctionBody(std::unique_ptr<SyntaxTree> param_list,
                     std::unique_ptr<SyntaxTree> block)
            : param_list_(std::move(param_list)), block_(std::move(block))
        {
        }

        virtual void Accept(Visitor *v);
    };
} // namespace luna

#endif // SYNTAX_TREE_H
