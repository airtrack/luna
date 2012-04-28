#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ParseTreeNode.h"
#include "LexTable.h"
#include <vector>

namespace lua
{
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

        TermExpression(TermType type, Value *value);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        TermType type_;
        Value *value_;
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

        BinaryExpression(BinaryType type, ExpressionPtr &&left_exp, ExpressionPtr &&right_exp);

        virtual void GenerateCode(CodeWriter *writer);

        static BinaryType GetBinaryType(TokenType type);

    private:
        void GenerateAnd(CodeWriter *writer);
        void GenerateOr(CodeWriter *writer);
        void GenerateJmp(CodeWriter *writer, OpCode op_code);
        void GenerateOther(CodeWriter *writer);
        void GenerateLeftExp(CodeWriter *writer);
        void GenerateRightExp(CodeWriter *writer);
        void GenerateOpInstruction(CodeWriter *writer);

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

        UnaryExpression(UnaryType type, ExpressionPtr &&exp);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        UnaryType type_;
        ExpressionPtr exp_;
    };

    enum TableKeyType
    {
        TableKeyType_Name,
        TableKeyType_Exp,
    };

    class TableKeyExpression : public Expression
    {
    public:
        TableKeyExpression(TableKeyType type, ExpressionPtr &&key);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        TableKeyType type_;
        ExpressionPtr key_;
    };

    class TableExpression : public Expression
    {
    public:
        TableExpression();

        void AddField(std::pair<ExpressionPtr, ExpressionPtr> &&field)
        {
            fields_.push_back(std::move(field));
        }

        virtual void GenerateCode(CodeWriter *writer);

    private:
        typedef std::vector<std::pair<ExpressionPtr, ExpressionPtr>> Fields;

        void GenerateField(CodeWriter *writer, Fields::iterator it, bool reset_value);

        Fields fields_;
        int array_index_helper_;
    };

    enum MemberType
    {
        MemberType_Dot,
        MemberType_Sub,
    };

    class MemberExpression : public Expression
    {
    public:
        MemberExpression(ExpressionPtr &&table,
                         ExpressionPtr &&member,
                         MemberType member_type);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr table_exp_;
        ExpressionPtr member_exp_;
        MemberType member_type_;
    };

    enum ParseNameType
    {
        ParseNameType_DefineLocalName,
        ParseNameType_GetName,
        ParseNameType_GetMemberName,
    };

    class NameExpression : public Expression
    {
    public:
        NameExpression(String *name, ParseNameType parse_name_type);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        void GenerateNameTypeCode(CodeWriter *writer);

        String *name_;
        ParseNameType parse_name_type_;
    };

    class NameListExpression : public Expression
    {
    public:
        std::size_t GetCount() const;
        void AddName(ExpressionPtr &&name);
        ExpressionPtr PopName();

        virtual void GenerateCode(CodeWriter *writer);

    private:
        std::vector<ExpressionPtr> name_list_;
    };

    class ExpListExpression : public Expression
    {
    public:
        std::size_t GetCount() const;
        void AddExp(ExpressionPtr &&exp);
        ExpressionPtr PopExp();

        virtual void GenerateCode(CodeWriter *writer);

    private:
        void PushEmptyCounter(CodeWriter *writer);
        void ExpListGenerateCode(CodeWriter *writer);

        std::vector<ExpressionPtr> exp_list_;
    };

    class VarListExpression : public Expression
    {
    public:
        std::size_t GetCount() const;
        void AddVar(ExpressionPtr &&var);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        std::vector<ExpressionPtr> var_list_;
    };

    class FuncNameExpression : public Expression
    {
    public:
        FuncNameExpression(ExpressionPtr &&pre_name, ExpressionPtr &&member);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr pre_name_;
        ExpressionPtr member_;
    };

    class ParamListExpression : public Expression
    {
    public:
        ParamListExpression(ExpressionPtr &&name_list, bool has_dot3);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr name_list_;
        bool has_dot3_;
    };

    class CallerExpression : public Expression
    {
    public:
        CallerExpression(ExpressionPtr &&caller, bool is_variable);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr caller_;
        bool is_variable_;
    };

    class FuncCallExpression : public Expression
    {
    public:
        FuncCallExpression(ExpressionPtr &&caller,
            ExpressionPtr &&member, ExpressionPtr &&arg_list);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr caller_;
        ExpressionPtr member_;
        ExpressionPtr arg_list_;
    };

    class AssignExpression : public Expression
    {
    public:
        AssignExpression(std::unique_ptr<VarListExpression> &&var_list,
                         std::unique_ptr<ExpListExpression> &&exp_list);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        std::unique_ptr<VarListExpression> var_list_;
        std::unique_ptr<ExpListExpression> exp_list_;
    };

    class FuncDefineExpression : public Expression
    {
    public:
        explicit FuncDefineExpression(StatementPtr &&func_def);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        StatementPtr func_def_;
    };

    enum PreExpType
    {
        PreExp_Variable,
        PreExp_FuncCall,
        PreExp_ParentheseExp,
    };

    class PreExpExpression : public Expression
    {
    public:
        PreExpExpression(ExpressionPtr &&exp, PreExpType type);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr exp_;
        PreExpType type_;
    };

    enum FuncNameType;

    ExpressionPtr ParseExpression(Lexer *lexer);
    ExpressionPtr ParseNameExpression(Lexer *lexer, ParseNameType type);
    ExpressionPtr ParseFuncNameExpression(Lexer *lexer, FuncNameType& type);
    ExpressionPtr ParseParamListExpression(Lexer *lexer);
    ExpressionPtr ParseFuncCallOrAssignExpression(Lexer *lexer);
    std::unique_ptr<NameListExpression> ParseNameListExpression(Lexer *lexer);
    std::unique_ptr<ExpListExpression> ParseExpListExpression(Lexer *lexer);
} // namespace lua

#endif // EXPRESSION_H
