#ifndef STATEMENT_H
#define STATEMENT_H

#include "ParseTreeNode.h"
#include <vector>

namespace lua
{
    class Statement : public ParseTreeNode
    {
    public:
    };

    typedef std::unique_ptr<Statement> StatementPtr;

    class BlockStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        std::vector<ParseTreeNodePtr> statements_;
        StatementPtr return_stat_;
    };

    class ChunkStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        StatementPtr block_stmt_;
    };

    class DoStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        StatementPtr block_stmt_;
    };

    class WhileStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        ParseTreeNodePtr exp_;
        StatementPtr block_stmt_;
    };

    class RepeatStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        StatementPtr block_stmt_;
        ParseTreeNodePtr exp_;
    };

    class IfStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        ParseTreeNodePtr exp_;
        StatementPtr block_stmt_;
        StatementPtr else_stmt_;
    };

    class ElseIfStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        ParseTreeNodePtr exp_;
        StatementPtr block_stmt_;
        StatementPtr else_stmt_;
    };

    class ElseStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        StatementPtr block_stmt_;
    };

    class ForStatement : public Statement
    {
    public:
        ForStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        bool in_mode_;
        ParseTreeNodePtr name_list_;
        ParseTreeNodePtr exp_list_;
        StatementPtr block_stmt_;
    };

    class FunctionStatement : public Statement
    {
    public:
        enum FuncNameType
        {
            NORMAL_FUNC_NAME,
            LOCAL_FUNC_NAME,
            NO_FUNC_NAME,
        };

        explicit FunctionStatement(FuncNameType name_type = NORMAL_FUNC_NAME);
        virtual bool ParseNode(Lexer *lexer);

    private:
        void ParseFuncName(Lexer *lexer);

        FuncNameType name_type_;
        ParseTreeNodePtr func_name_;
        ParseTreeNodePtr param_list_;
        StatementPtr block_stmt_;
    };

    class LocalStatement : public Statement
    {
    public:
        LocalStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        bool is_func_;
        StatementPtr func_stmt_;
        ParseTreeNodePtr name_list_;
        ParseTreeNodePtr exp_list_;
    };

    class BreakStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);
    };

    class ReturnStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);

    private:
        ParseTreeNodePtr exp_list_;
    };
} // namespace lua

#endif // STATEMENT_H
