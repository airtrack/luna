#ifndef STATEMENT_H
#define STATEMENT_H

#include "ParseTreeNode.h"
#include <vector>

namespace lua
{
    class Function;

    class BlockStatement : public Statement
    {
    public:
        void Add(StatementPtr &&statement)
        {
            statements_.push_back(std::move(statement));
        }

        virtual void GenerateCode(CodeWriter *writer);

    private:
        std::vector<StatementPtr> statements_;
    };

    StatementPtr ParseBlockStatement(Lexer *lexer);

    class NormalStatement : public Statement
    {
    public:
        explicit NormalStatement(ExpressionPtr &&exp);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr exp_;
    };

    StatementPtr ParseNormalStatement(Lexer *lexer);

    class DoStatement : public Statement
    {
    public:
        explicit DoStatement(StatementPtr &&block_stmt);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        StatementPtr block_stmt_;
    };

    StatementPtr ParseDoStatement(Lexer *lexer);

    class WhileStatement : public Statement
    {
    public:
        WhileStatement(ExpressionPtr &&exp, StatementPtr &&stmt);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr exp_;
        StatementPtr block_stmt_;
    };

    StatementPtr ParseWhileStatement(Lexer *lexer);

    class RepeatStatement : public Statement
    {
    public:
        RepeatStatement(StatementPtr &&stmt, ExpressionPtr &&exp);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        StatementPtr block_stmt_;
        ExpressionPtr exp_;
    };

    StatementPtr ParseRepeatStatement(Lexer *lexer);

    class IfStatement : public Statement
    {
    public:
        IfStatement(ExpressionPtr &&exp,
                    StatementPtr &&block_stmt,
                    StatementPtr &&else_stmt);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        void GenerateTrueBlock(CodeWriter *writer);
        void GenerateFalseBlock(CodeWriter *writer);

        ExpressionPtr exp_;
        StatementPtr block_stmt_;
        StatementPtr else_stmt_;
    };

    StatementPtr ParseIfStatement(Lexer *lexer);
    StatementPtr ParseElseIfStatement(Lexer *lexer);

    class ElseStatement : public Statement
    {
    public:
        explicit ElseStatement(StatementPtr &&stmt);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        StatementPtr block_stmt_;
    };

    StatementPtr ParseElseStatement(Lexer *lexer);

    class NumericForStatement : public Statement
    {
    public:
        NumericForStatement(ExpressionPtr &&var,
                            ExpressionPtr &&exp1,
                            ExpressionPtr &&limit,
                            ExpressionPtr &&exp2,
                            ExpressionPtr &&step,
                            ExpressionPtr &&exp3,
                            ExpressionPtr &&zero,
                            StatementPtr &&block_stmt);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        void GenerateInit(CodeWriter *writer);
        void GenerateAssign(CodeWriter *writer,
                            ExpressionPtr& exp,
                            ExpressionPtr& name);
        void GenerateCompare(CodeWriter *writer,
                             ExpressionPtr& name1,
                             ExpressionPtr& name2,
                             OpCode op_code);
        void GenerateCompareZero(CodeWriter *writer);
        void GenerateBody(CodeWriter *writer);

        ExpressionPtr var_;
        ExpressionPtr exp1_;
        ExpressionPtr limit_;
        ExpressionPtr exp2_;
        ExpressionPtr step_;
        ExpressionPtr exp3_;
        ExpressionPtr zero_;
        StatementPtr block_stmt_;
    };

    class GenericForStatement : public Statement
    {
    public:
        GenericForStatement(ExpressionPtr &&f,
                            ExpressionPtr &&s,
                            ExpressionPtr &&var,
                            ExpressionPtr &&name_list,
                            ExpressionPtr &&exp_list,
                            StatementPtr &&block_stmt);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        void GenerateInit(CodeWriter *writer);
        void GenerateCall(CodeWriter *writer);
        void GenerateAssignVar(CodeWriter *writer);

        ExpressionPtr f_;
        ExpressionPtr s_;
        ExpressionPtr var_;
        ExpressionPtr name_list_;
        ExpressionPtr exp_list_;
        StatementPtr block_stmt_;
    };

    StatementPtr ParseForStatement(Lexer *lexer);

    class FunctionStatement : public Statement
    {
    public:
        FunctionStatement(FuncNameType name_type,
                          String *self_name,
                          ExpressionPtr &&func_name,
                          ExpressionPtr &&param_list,
                          StatementPtr &&block_stmt,
                          Function *func);

        Function * GetFunction() const;

        virtual void GenerateCode(CodeWriter *writer);

    private:
        void GenerateFunctionCode(CodeWriter *writer);
        void GenerateSelfParam(CodeWriter *writer);
        void GenerateClosure(CodeWriter *writer);
        void GenerateFuncName(CodeWriter *writer);

        FuncNameType name_type_;
        String *self_name_;
        ExpressionPtr func_name_;
        ExpressionPtr param_list_;
        StatementPtr block_stmt_;
        Function *func_;
    };

    std::unique_ptr<FunctionStatement> ParseFunctionStatement(
        Lexer *lexer, FuncNameType type = NORMAL_FUNC_NAME);

    void CollectUpvalueFromFunc(
        Lexer *lexer, const std::unique_ptr<FunctionStatement>& func_stmt);

    class LocalStatement : public Statement
    {
    public:
        LocalStatement(ExpressionPtr &&name_list,
                       ExpressionPtr &&exp_list);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr name_list_;
        ExpressionPtr exp_list_;
    };

    StatementPtr ParseLocalStatement(Lexer *lexer);

    class BreakStatement : public Statement
    {
    public:
        virtual void GenerateCode(CodeWriter *writer) {}
    };

    StatementPtr ParseBreakStatement(Lexer *lexer);

    class ReturnStatement : public Statement
    {
    public:
        explicit ReturnStatement(ExpressionPtr &&return_exp_list);

        virtual void GenerateCode(CodeWriter *writer);

    private:
        ExpressionPtr return_exp_list_;
    };

    StatementPtr ParseReturnStatement(Lexer *lexer);

    StatementPtr ParseChunkStatement(Lexer *lexer);
} // namespace lua

#endif // STATEMENT_H
