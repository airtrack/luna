#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ParseTreeNode.h"
#include "LexTable.h"
#include <vector>

namespace lua
{
    class Expression : public ParseTreeNode
    {
    public:
        virtual bool ParseNode(Lexer *lexer) { return true; }
    };

    typedef std::unique_ptr<Expression> ExpressionPtr;

    class TermExpression : public Expression
    {
    public:
        enum TermType
        {
            TERM_NIL,
            TERM_FALSE,
            TERM_TRUE,
            TERM_NUMBER,
            TERM_STRING,
            TERM_PARAM_LIST,
        };

        TermExpression(TermType type, int index);

    private:
        TermType type_;
        int index_;
    };

    class BinaryExpression : public Expression
    {
    public:
        enum BinaryType
        {
            BINARY_TYPE_NONE,
            BINARY_TYPE_POWER,
            BINARY_TYPE_MULTIPLY,
            BINARY_TYPE_DIVIDE,
            BINARY_TYPE_MOD,
            BINARY_TYPE_PLUS,
            BINARY_TYPE_MINUS,
            BINARY_TYPE_CONCAT,
            BINARY_TYPE_LESS,
            BINARY_TYPE_GREATER,
            BINARY_TYPE_LESS_EQUAL,
            BINARY_TYPE_GREATER_EQUAL,
            BINARY_TYPE_NOT_EQUAL,
            BINARY_TYPE_EQUAL,
            BINARY_TYPE_AND,
            BINARY_TYPE_OR,
        };

        BinaryExpression(BinaryType type, ExpressionPtr left_exp, ExpressionPtr right_exp);

        static BinaryType GetBinaryType(TokenType type);

    private:
        BinaryType type_;
        ExpressionPtr left_exp_;
        ExpressionPtr right_exp_;
    };

    class UnaryExpression : public Expression
    {
    public:
        enum UnaryType
        {
            UNARY_TYPE_NOT,
            UNARY_TYPE_LENGTH,
            UNARY_TYPE_NEGATIVE,
        };

        UnaryExpression(UnaryType type, ExpressionPtr exp);

    private:
        UnaryType type_;
        ExpressionPtr exp_;
    };

    class TableFieldExpression : public Expression
    {
    public:
        TableFieldExpression(ExpressionPtr key, ExpressionPtr value);

    private:
        ExpressionPtr key_;
        ExpressionPtr value_;
    };

    class TableExpression : public Expression
    {
    public:
        void AddField(ExpressionPtr field)
        {
            fields_.push_back(std::move(field));
        }

    private:
        std::vector<ExpressionPtr> fields_;
    };

    class MemberExpression : public Expression
    {
    public:
        MemberExpression(ExpressionPtr table, ExpressionPtr member);

    private:
        ExpressionPtr table_exp_;
        ExpressionPtr member_exp_;
    };

    class NameExpression : public Expression
    {
    public:
        NameExpression(int index);

    private:
        int index_;
    };

    class NameListExpression : public Expression
    {
    public:
        std::size_t GetCount() const;
        void AddName(ExpressionPtr name);

    private:
        std::vector<ExpressionPtr> name_list_;
    };

    class ExpListExpression : public Expression
    {
    public:
        std::size_t GetCount() const;
        void AddExp(ExpressionPtr exp);

    private:
        std::vector<ExpressionPtr> exp_list_;
    };

    class VarListExpression : public Expression
    {
    public:
        std::size_t GetCount() const;
        void AddVar(ExpressionPtr var);

    private:
        std::vector<ExpressionPtr> var_list_;
    };

    class FuncNameExpression : public Expression
    {
    public:
        FuncNameExpression(ExpressionPtr pre_name, ExpressionPtr member);

    private:
        ExpressionPtr pre_name_;
        ExpressionPtr member_;
    };

    class ParamListExpression : public Expression
    {
    public:
        ParamListExpression(ExpressionPtr name_list, ExpressionPtr dot3);

    private:
        ExpressionPtr name_list_;
        ExpressionPtr dot3_;
    };

    class FuncCallExpression : public Expression
    {
    public:
        FuncCallExpression(ExpressionPtr caller,
            ExpressionPtr member, ExpressionPtr arg_list);

    private:
        ExpressionPtr caller_;
        ExpressionPtr member_;
        ExpressionPtr arg_list_;
    };

    class AssignExpression : public Expression
    {
    public:
        AssignExpression(ExpressionPtr var_list, ExpressionPtr exp_list);

    private:
        ExpressionPtr var_list_;
        ExpressionPtr exp_list_;
    };

    class FuncDefineExpression : public Expression
    {
    public:
        explicit FuncDefineExpression(ParseTreeNodePtr func_def);

    public:
        ParseTreeNodePtr func_def_;
    };

    ExpressionPtr ParseNameExpression(Lexer *lexer);
    std::unique_ptr<NameListExpression> ParseNameListExpression(Lexer *lexer);
    ExpressionPtr ParseExpression(Lexer *lexer);
    std::unique_ptr<ExpListExpression> ParseExpListExpression(Lexer *lexer);
    ExpressionPtr ParseFuncNameExpression(Lexer *lexer);
    ExpressionPtr ParseParamListExpression(Lexer *lexer);
    ExpressionPtr ParseFuncCallOrAssignExpression(Lexer *lexer);
} // namespace lua

#endif // EXPRESSION_H
