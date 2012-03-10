#include "Statement.h"
#include "Expression.h"
#include "Error.h"
#include "Lexer.h"

namespace lua
{
#define CASE_PARSE_STATEMENT(case_exp, parse_func)      \
    case case_exp:                                      \
        lexer->UngetToken(index);                       \
        return parse_func(lexer)

#define DEFAULT_PARSE_STATEMENT(parse_func)             \
    default:                                            \
        lexer->UngetToken(index);                       \
        return parse_func(lexer)

    StatementPtr ParseNonReturnStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        switch (lex_table[index]->type)
        {
        CASE_PARSE_STATEMENT(KW_DO, ParseDoStatement);
        CASE_PARSE_STATEMENT(KW_WHILE, ParseWhileStatement);
        CASE_PARSE_STATEMENT(KW_REPEAT, ParseRepeatStatement);
        CASE_PARSE_STATEMENT(KW_IF, ParseIfStatement);
        CASE_PARSE_STATEMENT(KW_FOR, ParseForStatement);
        CASE_PARSE_STATEMENT(KW_FUNCTION, ParseFunctionStatement);
        CASE_PARSE_STATEMENT(KW_LOCAL, ParseLocalStatement);
        CASE_PARSE_STATEMENT(KW_BREAK, ParseBreakStatement);
        DEFAULT_PARSE_STATEMENT(ParseNormalStatement);
        }
    }

    StatementPtr ParseStatement(Lexer *lexer, bool *is_return_stmt)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        while (index != -1 &&
               lex_table[index]->type != KW_END &&
               lex_table[index]->type != KW_UNTIL &&
               lex_table[index]->type != KW_ELSE &&
               lex_table[index]->type != KW_ELSEIF)
        {
            if (lex_table[index]->type == OP_SEMICOLON)
                index = lexer->GetToken();
            else if (lex_table[index]->type == KW_RETUREN)
            {
                lexer->UngetToken(index);
                if (is_return_stmt)
                    *is_return_stmt = true;
                return ParseReturnStatement(lexer);
            }
            else
            {
                lexer->UngetToken(index);
                return ParseNonReturnStatement(lexer);
            }
        }

        lexer->UngetToken(index);
        return StatementPtr();
    }

    void BlockStatement::GenerateCode(CodeWriter *writer)
    {
        for (auto it = statements_.begin(); it != statements_.end(); ++it)
            (*it)->GenerateCode(writer);
    }

    StatementPtr ParseBlockStatement(Lexer *lexer)
    {
        std::unique_ptr<BlockStatement> block_stmt(new BlockStatement);

        bool is_return_stmt = false;
        while (!is_return_stmt)
        {
            StatementPtr stmt = ParseStatement(lexer, &is_return_stmt);
            if (!stmt)
                break;
            block_stmt->Add(std::move(stmt));
        }

        return std::move(block_stmt);
    }

    ChunkStatement::ChunkStatement(StatementPtr &&block_stmt)
        : block_stmt_(std::move(block_stmt))
    {
    }

    void ChunkStatement::GenerateCode(CodeWriter *writer)
    {
        block_stmt_->GenerateCode(writer);
    }

    StatementPtr ParseChunkStatement(Lexer *lexer)
    {
        StatementPtr stmt = ParseBlockStatement(lexer);

        int index = lexer->GetToken();
        if (index != -1)
            THROW_PARSER_ERROR("expect 'eof' here");

        return StatementPtr(new ChunkStatement(std::move(stmt)));
    }

    NormalStatement::NormalStatement(ExpressionPtr &&exp)
        : exp_(std::move(exp))
    {
    }

    void NormalStatement::GenerateCode(CodeWriter *writer)
    {
        exp_->GenerateCode(writer);
    }

    StatementPtr ParseNormalStatement(Lexer *lexer)
    {
        ExpressionPtr exp = ParseFuncCallOrAssignExpression(lexer);
        return StatementPtr(new NormalStatement(std::move(exp)));
    }

    DoStatement::DoStatement(StatementPtr &&block_stmt)
        : block_stmt_(std::move(block_stmt))
    {
    }

    StatementPtr ParseDoBlockEnd(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        // Parse "do"
        if (index < 0 || lex_table[index]->type != KW_DO)
            THROW_PARSER_ERROR("expect 'do' here");

        // Parse the block statements
        StatementPtr block_stmt = ParseBlockStatement(lexer);

        // Parse "end"
        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");

        return block_stmt;
    }

    StatementPtr ParseDoStatement(Lexer *lexer)
    {
        StatementPtr stmt = ParseDoBlockEnd(lexer);
        return StatementPtr(new DoStatement(std::move(stmt)));
    }

    WhileStatement::WhileStatement(ExpressionPtr &&exp, StatementPtr &&stmt)
        : exp_(std::move(exp)),
          block_stmt_(std::move(stmt))
    {
    }

    StatementPtr ParseWhileStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_WHILE)
            THROW_PARSER_ERROR("expect 'while' here");

        ExpressionPtr exp = ParseExpression(lexer);
        StatementPtr stmt = ParseDoBlockEnd(lexer);

        return StatementPtr(new WhileStatement(std::move(exp), std::move(stmt)));
    }

    RepeatStatement::RepeatStatement(StatementPtr &&stmt, ExpressionPtr &&exp)
        : block_stmt_(std::move(stmt)),
          exp_(std::move(exp))
    {
    }

    StatementPtr ParseRepeatStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_REPEAT)
            THROW_PARSER_ERROR("expect 'repeat' here");

        StatementPtr stmt = ParseBlockStatement(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_UNTIL)
            THROW_PARSER_ERROR("expect 'until' here");

        ExpressionPtr exp = ParseExpression(lexer);

        return StatementPtr(new RepeatStatement(std::move(stmt), std::move(exp)));
    }

    void ParseExpThenElse(ExpressionPtr &exp, StatementPtr &block_stmt, StatementPtr &else_stmt, Lexer *lexer)
    {
        exp = ParseExpression(lexer);

        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_THEN)
            THROW_PARSER_ERROR("expect 'then' here");

        block_stmt = ParseBlockStatement(lexer);

        index = lexer->GetToken();
        if (index < 0)
            THROW_PARSER_ERROR("expect 'end' here");

        switch (lex_table[index]->type)
        {
        case KW_ELSEIF:
            lexer->UngetToken(index);
            else_stmt = ParseElseIfStatement(lexer);
            break;
        case KW_ELSE:
            lexer->UngetToken(index);
            else_stmt = ParseElseStatement(lexer);
            break;
        case KW_END:
            // No else statement
            break;
        }
    }

    IfStatement::IfStatement(ExpressionPtr &&exp,
                             StatementPtr &&block_stmt,
                             StatementPtr &&else_stmt)
        : exp_(std::move(exp)),
          block_stmt_(std::move(block_stmt)),
          else_stmt_(std::move(else_stmt))
    {
    }

    StatementPtr ParseIfStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_IF)
            THROW_PARSER_ERROR("expect 'if' here");

        ExpressionPtr exp;
        StatementPtr block_stmt;
        StatementPtr else_stmt;
        ParseExpThenElse(exp, block_stmt, else_stmt, lexer);

        return StatementPtr(new IfStatement(std::move(exp), std::move(block_stmt), std::move(else_stmt)));
    }

    ElseIfStatement::ElseIfStatement(ExpressionPtr &&exp,
                                     StatementPtr &&block_stmt,
                                     StatementPtr &&else_stmt)
        : exp_(std::move(exp)),
          block_stmt_(std::move(block_stmt)),
          else_stmt_(std::move(else_stmt))
    {
    }

    StatementPtr ParseElseIfStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_ELSEIF)
            THROW_PARSER_ERROR("expect 'elseif' here");

        ExpressionPtr exp;
        StatementPtr block_stmt;
        StatementPtr else_stmt;
        ParseExpThenElse(exp, block_stmt, else_stmt, lexer);

        return StatementPtr(new ElseIfStatement(std::move(exp), std::move(block_stmt), std::move(else_stmt)));
    }

    ElseStatement::ElseStatement(StatementPtr &&stmt)
        : block_stmt_(std::move(stmt))
    {
    }

    StatementPtr ParseElseStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_ELSE)
            THROW_PARSER_ERROR("expect 'else' here");

        StatementPtr stmt = ParseBlockStatement(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");

        return StatementPtr(new ElseStatement(std::move(stmt)));
    }

    ForStatement::ForStatement(bool in_mode, ExpressionPtr &&name_list,
            ExpressionPtr &&exp_list, StatementPtr &&block_stmt)
        : in_mode_(in_mode),
          name_list_(std::move(name_list)),
          exp_list_(std::move(exp_list)),
          block_stmt_(std::move(block_stmt))
    {
    }

    StatementPtr ParseForStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_FOR)
            THROW_PARSER_ERROR("expect 'for' here");

        std::unique_ptr<NameListExpression> name_list = ParseNameListExpression(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_ASSIGN && lex_table[index]->type != KW_IN)
            THROW_PARSER_ERROR("expect '=' or 'in' here");

        bool in_mode = false;
        if (lex_table[index]->type == KW_IN)
            in_mode = true;

        // '=' mode, then 'name' must only one.
        if (!in_mode && name_list->GetCount() != 1)
            THROW_PARSER_ERROR("expect only one 'name' here");

        std::unique_ptr<ExpListExpression> exp_list = ParseExpListExpression(lexer);
        if (!exp_list)
            THROW_PARSER_ERROR("expect expression here");

        // '=' mode, then 'exp' must less equal than three.
        if (!in_mode && exp_list->GetCount() > 3)
            THROW_PARSER_ERROR("expect three 'exp' here at most");

        StatementPtr block_stmt = ParseDoBlockEnd(lexer);

        return StatementPtr(new ForStatement(in_mode, std::move(name_list),
                    std::move(exp_list), std::move(block_stmt)));
    }

    FunctionStatement::FunctionStatement(FuncNameType name_type,
                                         ExpressionPtr &&func_name,
                                         ExpressionPtr &&param_list,
                                         StatementPtr &&block_stmt)
        : name_type_(name_type),
          func_name_(std::move(func_name)),
          param_list_(std::move(param_list)),
          block_stmt_(std::move(block_stmt))
    {
    }

    ExpressionPtr ParseFunctionName(Lexer *lexer, FuncNameType type)
    {
        switch (type)
        {
        case NORMAL_FUNC_NAME:
            return ParseFuncNameExpression(lexer);
        case LOCAL_FUNC_NAME:
            return ParseNameExpression(lexer);
        case NO_FUNC_NAME:
            // No func name, so we don't parse func name.
            break;
        }

        return ExpressionPtr();
    }

    StatementPtr ParseFunctionStatement(Lexer *lexer, FuncNameType type)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_FUNCTION)
            THROW_PARSER_ERROR("expect 'function' here");

        ExpressionPtr func_name = ParseFunctionName(lexer, type);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_LEFT_PARENTHESE)
            THROW_PARSER_ERROR("expect '(' here");

        ExpressionPtr param_list = ParseParamListExpression(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_RIGHT_PARENTHESE)
            THROW_PARSER_ERROR("expect ')' here");

        StatementPtr block_stmt = ParseBlockStatement(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");

        return StatementPtr(new FunctionStatement(type, std::move(func_name),
                    std::move(param_list), std::move(block_stmt)));
    }

    LocalStatement::LocalStatement(ExpressionPtr &&name_list,
                                   ExpressionPtr &&exp_list)
        : name_list_(std::move(name_list)),
          exp_list_(std::move(exp_list))
    {
    }

    StatementPtr ParseLocalFunctionStatement(Lexer *lexer)
    {
        return ParseFunctionStatement(lexer, LOCAL_FUNC_NAME);
    }

    StatementPtr ParseLocalNameListStatement(Lexer *lexer)
    {
        ExpressionPtr name_list = ParseNameListExpression(lexer);
        ExpressionPtr exp_list;

        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index >= 0 && lex_table[index]->type == OP_ASSIGN)
        {
            exp_list = ParseExpListExpression(lexer);
            if (!exp_list)
                THROW_PARSER_ERROR("expect expression here");
        }
        else
        {
            lexer->UngetToken(index);
        }

        return StatementPtr(new LocalStatement(std::move(name_list), std::move(exp_list)));
    }

    StatementPtr ParseLocalStatement(Lexer *lexer)
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
            lexer->UngetToken(index);
            return ParseLocalFunctionStatement(lexer);
        }
        else
        {
            lexer->UngetToken(index);
            return ParseLocalNameListStatement(lexer);
        }
    }

    StatementPtr ParseBreakStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_BREAK)
            THROW_PARSER_ERROR("expect 'break' here");

        return StatementPtr(new BreakStatement());
    }

    ReturnStatement::ReturnStatement(ExpressionPtr &&return_exp_list)
        : return_exp_list_(std::move(return_exp_list))
    {
    }

    StatementPtr ParseReturnStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_RETUREN)
            THROW_PARSER_ERROR("expect 'return' here");

        ExpressionPtr exp_list = ParseExpListExpression(lexer);

        // Parse "[;]"
        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_SEMICOLON)
            lexer->UngetToken(index);

        return StatementPtr(new ReturnStatement(std::move(exp_list)));
    }
} // namespace lua
