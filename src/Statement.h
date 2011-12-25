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

    class BlockStatement : public Statement
    {
    public:
        BlockStatement();
        virtual ~BlockStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        std::vector<Statement *> statements_;
        Statement *return_stat_;
    };

    class ChunkStatement : public Statement
    {
    public:
        ChunkStatement();
        virtual ~ChunkStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        Statement *block_stmt_;
    };

    class DoStatement : public Statement
    {
    public:
        DoStatement();
        virtual ~DoStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        Statement *block_stmt_;
    };

    class WhileStatement : public Statement
    {
    public:
        WhileStatement();
        virtual ~WhileStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        ParseTreeNode *exp_;
        Statement *block_stmt_;
    };

    class RepeatStatement : public Statement
    {
    public:
        RepeatStatement();
        virtual ~RepeatStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        Statement *block_stmt_;
        ParseTreeNode *exp_;
    };

    class IfStatement : public Statement
    {
    public:
        IfStatement();
        virtual ~IfStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        ParseTreeNode *exp_;
        Statement *block_stmt_;
        Statement *else_stmt_;
    };

    class ElseIfStatement : public Statement
    {
    public:
        ElseIfStatement();
        virtual ~ElseIfStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        ParseTreeNode *exp_;
        Statement *block_stmt_;
        Statement *else_stmt_;
    };

    class ElseStatement : public Statement
    {
    public:
        ElseStatement();
        virtual ~ElseStatement();
        virtual bool ParseNode(Lexer *lexer);

    private:
        Statement *block_stmt_;
    };

    class ForStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);
    };

    class FunctionStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);
    };

    class LocalStatement : public Statement
    {
    public:
        virtual bool ParseNode(Lexer *lexer);
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
    };
} // namespace lua

#endif // STATEMENT_H
