#include "Statement.h"
#include "Expression.h"
#include "Error.h"
#include "Lexer.h"

namespace lua
{
    bool ParseDoBlockEnd(StatementPtr &block_stmt, Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        // Parse "do"
        if (index < 0 || lex_table[index]->type != KW_DO)
            THROW_PARSER_ERROR("expect 'do' here");

        // Parse the block statements
        block_stmt.reset(new BlockStatement);
        block_stmt->ParseNode(lexer);

        // Parse "end"
        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");

        return true;
    }

    bool BlockStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        while (index != -1 && return_stat_ != 0)
        {
            ParseTreeNodePtr stat;
            switch (lex_table[index]->type)
            {
            case OP_SEMICOLON:
                // Empty statement
                break;
            case KW_DO:
                stat.reset(new DoStatement);
                break;
            case KW_WHILE:
                stat.reset(new WhileStatement);
                break;
            case KW_REPEAT:
                stat.reset(new RepeatStatement);
                break;
            case KW_IF:
                stat.reset(new IfStatement);
                break;
            case KW_FOR:
                stat.reset(new ForStatement);
                break;
            case KW_FUNCTION:
                stat.reset(new FunctionStatement);
                break;
            case KW_LOCAL:
                stat.reset(new LocalStatement);
                break;
            case KW_BREAK:
                stat.reset(new BreakStatement);
                break;
            case KW_RETUREN:
                return_stat_.reset(new ReturnStatement);
                lexer->UngetToken(index);
                return_stat_->ParseNode(lexer);
                break;
            case KW_END:
            case KW_UNTIL:
            case KW_ELSE:
            case KW_ELSEIF:
                // End of the block
                lexer->UngetToken(index);
                return true;
            default:
                stat = ParseFuncCallOrAssignExpression(lexer);
                break;
            }

            if (stat)
            {
                lexer->UngetToken(index);
                stat->ParseNode(lexer);
                statements_.push_back(std::move(stat));
            }

            index = lexer->GetToken();
        }

        lexer->UngetToken(index);
        return true;
    }

    bool ChunkStatement::ParseNode(Lexer *lexer)
    {
        block_stmt_.reset(new BlockStatement);
        block_stmt_->ParseNode(lexer);

        int index = lexer->GetToken();
        if (index != -1)
            THROW_PARSER_ERROR("expect '<eof>' here");

        return true;
    }

    bool DoStatement::ParseNode(Lexer *lexer)
    {
        return ParseDoBlockEnd(block_stmt_, lexer);
    }

    bool WhileStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_WHILE)
            THROW_PARSER_ERROR("expect 'while' here");

        exp_ = ParseExpression(lexer);

        return ParseDoBlockEnd(block_stmt_, lexer);
    }

    bool RepeatStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_REPEAT)
            THROW_PARSER_ERROR("expect 'repeat' here");

        block_stmt_.reset(new BlockStatement);
        block_stmt_->ParseNode(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_UNTIL)
            THROW_PARSER_ERROR("expect 'until' here");

        exp_ = ParseExpression(lexer);
        return true;
    }

    bool ParseExpThenElse(ParseTreeNodePtr &exp, StatementPtr &block_stmt, StatementPtr &else_stmt, Lexer *lexer)
    {
        exp = ParseExpression(lexer);

        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_THEN)
            THROW_PARSER_ERROR("expect 'then' here");

        block_stmt.reset(new BlockStatement);
        block_stmt->ParseNode(lexer);

        index = lexer->GetToken();
        if (index < 0)
            THROW_PARSER_ERROR("expect 'end' here");

        switch (lex_table[index]->type)
        {
        case KW_ELSEIF:
            else_stmt.reset(new ElseIfStatement);
            break;
        case KW_ELSE:
            else_stmt.reset(new ElseStatement);
            break;
        case KW_END:
            // No else statement, so else_stmt_ is 0.
            return true;
        }

        if (else_stmt)
        {
            lexer->UngetToken(index);
            else_stmt->ParseNode(lexer);
        }
        return true;
    }

    bool IfStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_IF)
            THROW_PARSER_ERROR("expect 'if' here");

        return ParseExpThenElse(exp_, block_stmt_, else_stmt_, lexer);
    }

    bool ElseIfStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_ELSEIF)
            THROW_PARSER_ERROR("expect 'elseif' here");

        return ParseExpThenElse(exp_, block_stmt_, else_stmt_, lexer);
    }

    bool ElseStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_ELSE)
            THROW_PARSER_ERROR("expect 'else' here");

        block_stmt_.reset(new BlockStatement);
        block_stmt_->ParseNode(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");
        return true;
    }

    ForStatement::ForStatement()
        : in_mode_(false)
    {
    }

    bool ForStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_FOR)
            THROW_PARSER_ERROR("expect 'for' here");

        std::unique_ptr<NameListExpression> name_list = ParseNameListExpression(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_ASSIGN ||
            lex_table[index]->type != KW_IN)
            THROW_PARSER_ERROR("expect '=' or 'in' here");

        if (lex_table[index]->type == KW_IN)
            in_mode_ = true;

        // '=' mode, then 'name' must only one.
        if (!in_mode_ && name_list->GetCount() != 1)
            THROW_PARSER_ERROR("expect only one 'name' here");

        std::unique_ptr<ExpListExpression> exp_list = ParseExpListExpression(lexer);

        // '=' mode, then 'exp' must less equal than three.
        if (!in_mode_ && exp_list->GetCount() > 3)
            THROW_PARSER_ERROR("expect three 'exp' here at most");

        ParseDoBlockEnd(block_stmt_, lexer);
        name_list_ = name_list;
        exp_list_ = exp_list;
        return true;
    }

    FunctionStatement::FunctionStatement(FuncNameType name_type)
        : name_type_(name_type)
    {
    }

    bool FunctionStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_FUNCTION)
            THROW_PARSER_ERROR("expect 'function' here");

        ParseFuncName(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_LEFT_PARENTHESE)
            THROW_PARSER_ERROR("expect '(' here");

        param_list_ = ParseParamListExpression(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_RIGHT_PARENTHESE)
            THROW_PARSER_ERROR("expect ')' here");

        block_stmt_.reset(new BlockStatement);
        block_stmt_->ParseNode(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");
        return true;
    }

    void FunctionStatement::ParseFuncName(Lexer *lexer)
    {
        switch (name_type_)
        {
        case NORMAL_FUNC_NAME:
            func_name_ = ParseFuncNameExpression(lexer);
            break;
        case LOCAL_FUNC_NAME:
            func_name_ = ParseNameExpression(lexer);
            break;
        case NO_FUNC_NAME:
            // No func name, so we don't parse func name.
            break;
        }
    }

    LocalStatement::LocalStatement()
        : is_func_(false)
    {
    }

    bool LocalStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_LOCAL)
            THROW_PARSER_ERROR("expect 'local' here");

        index = lexer->GetToken();
        if (index < 0)
            THROW_PARSER_ERROR("expect 'function' or 'name' here");

        if (lex_table[index]->type == KW_FUNCTION)
        {
            is_func_ = true;
            lexer->UngetToken(index);
            func_stmt_.reset(new FunctionStatement(FunctionStatement::LOCAL_FUNC_NAME));
            func_stmt_->ParseNode(lexer);
        }
        else
        {
            lexer->UngetToken(index);
            name_list_ = ParseNameListExpression(lexer);
            index = lexer->GetToken();
            if (index >= 0 && lex_table[index]->type == OP_ASSIGN)
            {
                exp_list_ = ParseExpListExpression(lexer);
            }
            else
            {
                lexer->UngetToken(index);
            }
        }

        return true;
    }

    bool BreakStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_BREAK)
            THROW_PARSER_ERROR("expect 'break' here");
        return true;
    }

    bool ReturnStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_RETUREN)
            THROW_PARSER_ERROR("expect 'return' here");

        exp_list_ = ParseExpListExpression(lexer);
        return true;
    }
} // namespace lua
