#include "Statement.h"
#include "Expression.h"
#include "StlUtility.h"
#include "Error.h"
#include "Lexer.h"

namespace lua
{
#define THROW_PARSER_ERROR(desc) \
    ParserError::ThrowError(     \
        lexer->GetLineNumber(),  \
        lexer->GetColumnNumber(),\
        desc)

    bool ParseDoBlockEnd(Statement *&block_stmt, Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetCurToken();

        // Parse "do"
        if (index < 0 || lex_table[index]->type != KW_DO)
            THROW_PARSER_ERROR("expect 'do' here");

        // Parse the block statements
        block_stmt = new BlockStatement;
        block_stmt->ParseNode(lexer);

        // Parse "end"
        index = lexer->GetCurToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");

        return true;
    }

    BlockStatement::BlockStatement()
        : return_stat_(0)
    {
    }

    BlockStatement::~BlockStatement()
    {
        erase_elements(statements_);
        delete return_stat_;
    }

    bool BlockStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetToken();

        while (index != -1 && return_stat_ != 0)
        {
            Statement *stat = 0;
            switch (lex_table[index]->type)
            {
            case OP_SEMICOLON:
                // Empty statement
                break;
            case KW_DO:
                stat = new DoStatement;
                break;
            case KW_WHILE:
                stat = new WhileStatement;
                break;
            case KW_REPEAT:
                stat = new RepeatStatement;
                break;
            case KW_IF:
                stat = new IfStatement;
                break;
            case KW_FOR:
                stat = new ForStatement;
                break;
            case KW_FUNCTION:
                stat = new FunctionStatement;
                break;
            case KW_LOCAL:
                stat = new LocalStatement;
                break;
            case KW_BREAK:
                stat = new BreakStatement;
                break;
            case KW_RETUREN:
                return_stat_ = new ReturnStatement;
                return_stat_->ParseNode(lexer);
                break;
            case KW_END:
            case KW_UNTIL:
            case KW_ELSE:
            case KW_ELSEIF:
                // End of the block
                return true;
            default:
                break;
            }

            if (stat)
            {
                statements_.push_back(stat);
                stat->ParseNode(lexer);
            }

            index = lexer->GetToken();
        }

        return true;
    }

    ChunkStatement::ChunkStatement()
        : block_stmt_(0)
    {
    }

    ChunkStatement::~ChunkStatement()
    {
        delete block_stmt_;
    }

    bool ChunkStatement::ParseNode(Lexer *lexer)
    {
        block_stmt_ = new BlockStatement;
        block_stmt_->ParseNode(lexer);

        int index = lexer->GetCurToken();
        if (index != -1)
            THROW_PARSER_ERROR("expect '<eof>' here");

        return true;
    }

    DoStatement::DoStatement()
        : block_stmt_(0)
    {
    }

    DoStatement::~DoStatement()
    {
        delete block_stmt_;
    }

    bool DoStatement::ParseNode(Lexer *lexer)
    {
        return ParseDoBlockEnd(block_stmt_, lexer);
    }

    WhileStatement::WhileStatement()
        : exp_(0),
          block_stmt_(0)
    {
    }

    WhileStatement::~WhileStatement()
    {
        delete exp_;
        delete block_stmt_;
    }

    bool WhileStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetCurToken();

        if (index < 0 || lex_table[index]->type != KW_WHILE)
            THROW_PARSER_ERROR("expect 'while' here");

        exp_ = new BasicExpression;
        exp_->ParseNode(lexer);

        return ParseDoBlockEnd(block_stmt_, lexer);
    }

    RepeatStatement::RepeatStatement()
        : block_stmt_(0),
          exp_(0)
    {
    }

    RepeatStatement::~RepeatStatement()
    {
        delete block_stmt_;
        delete exp_;
    }

    bool RepeatStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetCurToken();

        if (index < 0 || lex_table[index]->type != KW_REPEAT)
            THROW_PARSER_ERROR("expect 'repeat' here");

        block_stmt_ = new BlockStatement;
        block_stmt_->ParseNode(lexer);

        index = lexer->GetCurToken();
        if (index < 0 || lex_table[index]->type != KW_UNTIL)
            THROW_PARSER_ERROR("expect 'until' here");

        exp_ = new BasicExpression;
        return exp_->ParseNode(lexer);
    }

    bool ParseExpThenElse(ParseTreeNode *&exp, Statement *&block_stmt, Statement *&else_stmt, Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        exp = new BasicExpression;
        exp->ParseNode(lexer);

        int index = lexer->GetCurToken();
        if (index < 0 || lex_table[index]->type != KW_THEN)
            THROW_PARSER_ERROR("expect 'then' here");

        block_stmt = new BlockStatement;
        block_stmt->ParseNode(lexer);

        index = lexer->GetCurToken();
        if (index < 0)
            THROW_PARSER_ERROR("expect 'end' here");

        switch (lex_table[index]->type)
        {
        case KW_ELSEIF:
            else_stmt = new ElseIfStatement;
            break;
        case KW_ELSE:
            else_stmt = new ElseStatement;
            break;
        case KW_END:
            // No else statement, so else_stmt_ is 0.
            return true;
        }

        if (else_stmt)
            else_stmt->ParseNode(lexer);
        return true;
    }

    IfStatement::IfStatement()
        : exp_(0),
          block_stmt_(0),
          else_stmt_(0)
    {
    }

    IfStatement::~IfStatement()
    {
        delete exp_;
        delete block_stmt_;
        delete else_stmt_;
    }

    bool IfStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetCurToken();

        if (index < 0 || lex_table[index]->type != KW_IF)
            THROW_PARSER_ERROR("expect 'if' here");

        return ParseExpThenElse(exp_, block_stmt_, else_stmt_, lexer);
    }

    ElseIfStatement::ElseIfStatement()
        : exp_(0),
          block_stmt_(0),
          else_stmt_(0)
    {
    }

    ElseIfStatement::~ElseIfStatement()
    {
        delete exp_;
        delete block_stmt_;
        delete else_stmt_;
    }

    bool ElseIfStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetCurToken();

        if (index < 0 || lex_table[index]->type != KW_ELSEIF)
            THROW_PARSER_ERROR("expect 'elseif' here");

        return ParseExpThenElse(exp_, block_stmt_, else_stmt_, lexer);
    }

    ElseStatement::ElseStatement()
        : block_stmt_(0)
    {
    }

    ElseStatement::~ElseStatement()
    {
        delete block_stmt_;
    }

    bool ElseStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetCurToken();

        if (index < 0 || lex_table[index]->type != KW_ELSE)
            THROW_PARSER_ERROR("expect 'else' here");

        block_stmt_ = new BlockStatement;
        block_stmt_->ParseNode(lexer);

        index = lexer->GetCurToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");
        return true;
    }

    ForStatement::ForStatement()
        : in_mode_(false),
          name_list_(0),
          exp_list_(0),
          block_stmt_(0)
    {
    }

    ForStatement::~ForStatement()
    {
        delete name_list_;
        delete exp_list_;
        delete block_stmt_;
    }

    bool ForStatement::ParseNode(Lexer *lexer)
    {
        LexTable &lex_table = *(lexer->GetLexTable());
        int index = lexer->GetCurToken();

        if (index < 0 || lex_table[index]->type != KW_FOR)
            THROW_PARSER_ERROR("expect 'for' here");

        std::unique_ptr<NameListExpression> name_list(new NameListExpression);
        name_list->ParseNode(lexer);

        index = lexer->GetCurToken();
        if (index < 0 || lex_table[index]->type != OP_ASSIGN ||
            lex_table[index]->type != KW_IN)
            THROW_PARSER_ERROR("expect '=' or 'in' here");

        if (lex_table[index]->type == KW_IN)
            in_mode_ = true;

        // '=' mode, then 'name' must only one.
        if (!in_mode_ && name_list->GetNameCount() != 1)
            THROW_PARSER_ERROR("expect only one 'name' here");

        std::unique_ptr<ExpListExpression> exp_list(new ExpListExpression);
        exp_list->ParseNode(lexer);

        // '=' mode, then 'exp' must less equal than three.
        if (!in_mode_ && exp_list->GetExpCount() > 3)
            THROW_PARSER_ERROR("expect 3 'exp' here at most");

        ParseDoBlockEnd(block_stmt_, lexer);
        name_list_ = name_list.release();
        exp_list_ = exp_list.release();
        return true;
    }

    bool FunctionStatement::ParseNode(Lexer *lexer)
    {
        return true;
    }

    bool LocalStatement::ParseNode(Lexer *lexer)
    {
        return true;
    }

    bool BreakStatement::ParseNode(Lexer *lexer)
    {
        return true;
    }

    bool ReturnStatement::ParseNode(Lexer *lexer)
    {
        return true;
    }
} // namespace lua
