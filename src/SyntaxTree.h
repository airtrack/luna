#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "Token.h"
#include "Visitor.h"
#include <memory>
#include <vector>

namespace luna
{
    class SyntaxTree
    {
    public:
        virtual ~SyntaxTree() { }
        virtual void Accept(Visitor *v) = 0;
    };

    class Block : public SyntaxTree
    {
    public:
        std::vector<std::unique_ptr<SyntaxTree>> statements_;
        std::unique_ptr<SyntaxTree> return_stmt_;

        Block() { }

        virtual void Accept(Visitor *v);
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

    class ParamList : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> name_list_;
        bool vararg_;

        ParamList(std::unique_ptr<SyntaxTree> name_list, bool vararg)
            : name_list_(std::move(name_list)), vararg_(vararg)
        {
        }

        virtual void Accept(Visitor *v);
    };

    class NameList : public SyntaxTree
    {
    public:
        std::vector<TokenDetail> names_;

        NameList() { }

        virtual void Accept(Visitor *v);
    };

    class TableDefine : public SyntaxTree
    {
    public:
        std::vector<std::unique_ptr<SyntaxTree>> fields_;

        TableDefine() { }

        virtual void Accept(Visitor *v);
    };

    class TableIndexField : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> index_;
        std::unique_ptr<SyntaxTree> value_;

        TableIndexField(std::unique_ptr<SyntaxTree> index,
                        std::unique_ptr<SyntaxTree> value)
            : index_(std::move(index)), value_(std::move(value))
        {
        }

        virtual void Accept(Visitor *v);
    };

    class TableNameField : public SyntaxTree
    {
    public:
        TokenDetail name_;
        std::unique_ptr<SyntaxTree> value_;

        TableNameField(const TokenDetail &name,
                       std::unique_ptr<SyntaxTree> value)
            : name_(name), value_(std::move(value))
        {
        }

        virtual void Accept(Visitor *v);
    };

    class TableArrayField : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> value_;

        explicit TableArrayField(std::unique_ptr<SyntaxTree> value)
            : value_(std::move(value))
        {
        }

        virtual void Accept(Visitor *v);
    };

    class IndexAccessor : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> table_;
        std::unique_ptr<SyntaxTree> index_;

        IndexAccessor(std::unique_ptr<SyntaxTree> table,
                      std::unique_ptr<SyntaxTree> index)
            : table_(std::move(table)), index_(std::move(index))
        {
        }

        virtual void Accept(Visitor *v);
    };

    class MemberAccessor : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> table_;
        TokenDetail member_;

        MemberAccessor(std::unique_ptr<SyntaxTree> table,
                       const TokenDetail &member)
            : table_(std::move(table)), member_(member)
        {
        }

        virtual void Accept(Visitor *v);
    };

    class NormalFuncCall : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> caller_;
        std::unique_ptr<SyntaxTree> args_;

        NormalFuncCall(std::unique_ptr<SyntaxTree> caller,
                       std::unique_ptr<SyntaxTree> args)
            : caller_(std::move(caller)), args_(std::move(args))
        {
        }

        virtual void Accept(Visitor *v);
    };

    class MemberFuncCall : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> caller_;
        TokenDetail member_;
        std::unique_ptr<SyntaxTree> args_;

        MemberFuncCall(std::unique_ptr<SyntaxTree> caller,
                       const TokenDetail &member,
                       std::unique_ptr<SyntaxTree> args)
            : caller_(std::move(caller)), member_(member), args_(std::move(args))
        {
        }

        virtual void Accept(Visitor *v);
    };

    class ExpressionList : public SyntaxTree
    {
    public:
        std::vector<std::unique_ptr<SyntaxTree>> exp_list_;

        virtual void Accept(Visitor *v);
    };
} // namespace luna

#endif // SYNTAX_TREE_H
