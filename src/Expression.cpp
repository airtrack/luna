#include "Expression.h"
#include "Error.h"
#include "Lexer.h"

namespace lua
{
    TermExpression::TermExpression(TermType type, int index)
        : type_(type),
          index_(index)
    {
    }

    BinaryExpression::BinaryExpression(BinaryType type, ExpressionPtr left_exp, ExpressionPtr right_exp)
        : type_(type),
          left_exp_(std::move(left_exp)),
          right_exp_(std::move(right_exp))
    {
    }

    UnaryExpression::UnaryExpression(UnaryType type, ExpressionPtr exp)
        : type_(type),
          exp_(std::move(exp))
    {
    }

    TableFieldExpression::TableFieldExpression(ExpressionPtr key, ExpressionPtr value)
        : key_(std::move(key)),
          value_(std::move(value))
    {
    }

    MemberExpression::MemberExpression(ExpressionPtr table, ExpressionPtr member)
        : table_exp_(std::move(table)),
          member_exp_(std::move(member))
    {
    }

    NameExpression::NameExpression(int index)
        : index_(index)
    {
    }

    FuncNameExpression::FuncNameExpression(ExpressionPtr pre_name, ExpressionPtr member)
        : pre_name_(std::move(pre_name)),
          member_(std::move(member))
    {
    }

    ParamListExpression::ParamListExpression(ExpressionPtr name_list, ExpressionPtr dot3)
        : name_list_(std::move(name_list)),
          dot3_(std::move(dot3))
    {
    }

    FuncCallExpression::FuncCallExpression(ExpressionPtr caller,
        ExpressionPtr member, ExpressionPtr arg_list)
        : caller_(std::move(caller)),
          member_(std::move(member)),
          arg_list_(std::move(arg_list))
    {
    }

    AssignExpression::AssignExpression(ExpressionPtr var_list, ExpressionPtr exp_list)
        : var_list_(std::move(var_list)),
          exp_list_(std::move(exp_list))
    {
    }

    ExpressionPtr ParseDotMemberExpression(Lexer *lexer)
    {
        ExpressionPtr member = ParseNameExpression(lexer);
        if (!member)
            THROW_PARSER_ERROR("expect 'id' here");
        return member;
    }

    ExpressionPtr ParseSubMemberExpression(Lexer *lexer)
    {
        ExpressionPtr member = ParseBasicExpression(lexer);
        if (!member)
            THROW_PARSER_ERROR("expect 'expression' here");

        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_RIGHT_BRACKET)
            THROW_PARSER_ERROR("expect ']' here");

        return member;
    }

    ExpressionPtr ParseMemberExpression(ExpressionPtr table, Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != OP_DOT ||
            lex_table[index]->type != OP_LEFT_BRACKET)
        {
            lexer->UngetToken(index);
            return std::move(table);
        }

        ExpressionPtr member;
        if (lex_table[index]->type == OP_DOT)
            member = ParseDotMemberExpression(lexer);
        else
            member = ParseSubMemberExpression(lexer);

        table.reset(new MemberExpression(std::move(table), std::move(member)));
        return ParseMemberExpression(std::move(table), lexer);
    }

    ExpressionPtr ParseNameExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != IDENTIFIER)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        return ExpressionPtr(new NameExpression(index));
    }

    std::unique_ptr<NameListExpression> ParseNameListExpression(Lexer *lexer)
    {
        ExpressionPtr name = ParseNameExpression(lexer);
        if (!name)
            THROW_PARSER_ERROR("expect 'name' here");

        std::unique_ptr<NameListExpression> name_list(new NameListExpression);

        LexTable &lex_table = lexer->GetLexTable();
        while (true)
        {
            name_list->AddName(std::move(name));

            int index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_COMMA)
            {
                lexer->UngetToken(index);
                break;
            }

            name = ParseNameExpression(lexer);
            if (!name)
                THROW_PARSER_ERROR("expect 'name' here");
        }

        return name_list;
    }

    ExpressionPtr ParseBasicExpression(Lexer *lexer)
    {
        return ExpressionPtr();
    }

    std::unique_ptr<ExpListExpression> ParseExpListExpression(Lexer *lexer)
    {
        std::unique_ptr<ExpListExpression> exp_list;
        ExpressionPtr exp = ParseBasicExpression(lexer);
        if (!exp)
            return exp_list;

        exp_list.reset(new ExpListExpression);
        LexTable &lex_table = lexer->GetLexTable();

        while (true)
        {
            exp_list->AddExp(std::move(exp));

            int index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_COMMA)
            {
                lexer->UngetToken(index);
                break;
            }

            exp = ParseBasicExpression(lexer);
            if (!exp)
                THROW_PARSER_ERROR("expect expression here");
        }

        return exp_list;
    }

    ExpressionPtr ParseFuncNameExpression(Lexer *lexer)
    {
        ExpressionPtr name = ParseNameExpression(lexer);
        if (!name)
            THROW_PARSER_ERROR("expect 'name' here");

        name = ParseMemberExpression(std::move(name), lexer);

        ExpressionPtr member;
        LexTable &lex_table = lexer->GetLexTable();

        int index = lexer->GetToken();
        if (index >= 0 && lex_table[index]->type == OP_COLON)
        {
            member = ParseNameExpression(lexer);
            if (!member)
                THROW_PARSER_ERROR("expect 'name' here");
        }
        else
        {
            lexer->UngetToken(index);
        }

        return ExpressionPtr(new FuncNameExpression(std::move(name), std::move(member)));
    }

    ExpressionPtr ParseParamNameListExpression(Lexer *lexer)
    {
        std::unique_ptr<NameListExpression> name_list;
        ExpressionPtr name = ParseNameExpression(lexer);
        if (!name)
            return std::move(name_list);

        name_list.reset(new NameListExpression);

        LexTable &lex_table = lexer->GetLexTable();
        while (true)
        {
            name_list->AddName(std::move(name));

            int index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_COMMA)
            {
                lexer->UngetToken(index);
                break;
            }

            name = ParseNameExpression(lexer);
            if (!name)
            {
                lexer->UngetToken(index);
                break;
            }
        }

        return std::move(name_list);
    }

    ExpressionPtr ParseParamDot3Expression(bool need_comma, Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        ExpressionPtr dot3;

        int index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_COMMA ||
            lex_table[index]->type != OP_PARAM_LIST)
        {
            lexer->UngetToken(index);
            return dot3;
        }

        if (need_comma)
        {
            if (lex_table[index]->type == OP_COMMA)
                index = lexer->GetToken();
            else
                THROW_PARSER_ERROR("expect ',' here");
        }

        if (index < 0 || lex_table[index]->type != OP_PARAM_LIST)
            THROW_PARSER_ERROR("expect 'name' or '...' here");

        dot3.reset(new TermExpression(TermExpression::TERM_PARAM_LIST, index));
        return dot3;
    }

    ExpressionPtr ParseParamListExpression(Lexer *lexer)
    {
        ExpressionPtr name_list = ParseParamNameListExpression(lexer);
        ExpressionPtr dot3 = ParseParamDot3Expression(name_list, lexer);
        ExpressionPtr param_list;

        if (name_list || dot3)
            param_list.reset(new ParamListExpression(std::move(name_list), std::move(dot3)));

        return param_list;
    }

    ExpressionPtr ParseFuncCallMemberExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0)
            THROW_PARSER_ERROR("unexpect 'eof' here");

        ExpressionPtr member;
        if (lex_table[index]->type == OP_COLON)
        {
            member = ParseNameExpression(lexer);
            if (!member)
                THROW_PARSER_ERROR("expect 'id' here");
        }
        else
        {
            lexer->UngetToken(index);
        }

        return member;
    }

    ExpressionPtr ParseTableFieldKeyExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        ExpressionPtr key;
        if (index < 0 || lex_table[index]->type != OP_LEFT_BRACKET ||
            lex_table[index]->type != IDENTIFIER)
        {
            lexer->UngetToken(index);
            return key;
        }

        if (lex_table[index]->type == OP_LEFT_BRACKET)
        {
            key = ParseBasicExpression(lexer);
            if (!key)
                THROW_PARSER_ERROR("expect 'exp' here");
            index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_RIGHT_BRACKET)
                THROW_PARSER_ERROR("expect ']' here");
            index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_ASSIGN)
                THROW_PARSER_ERROR("expect '=' here");
        }
        else if (lex_table[index]->type == IDENTIFIER)
        {
            int i = lexer->GetToken();
            if (i < 0 || lex_table[i]->type != OP_ASSIGN)
            {
                lexer->UngetToken(i);
                lexer->UngetToken(index);
            }
            else
            {
                key.reset(new NameExpression(index));
            }
        }

        return key;
    }

    ExpressionPtr ParseTableFieldValueExpression(Lexer *lexer)
    {
        return ParseBasicExpression(lexer);
    }

    ExpressionPtr ParseTableFieldExpression(Lexer *lexer)
    {
        ExpressionPtr key = ParseTableFieldKeyExpression(lexer);
        ExpressionPtr value = ParseTableFieldValueExpression(lexer);

        if (value)
            return ExpressionPtr(new TableFieldExpression(std::move(key), std::move(value)));
        else
            return ExpressionPtr();
    }

    bool ParseTableFieldSeparator(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0)
            THROW_PARSER_ERROR("unexpect 'eof' here");

        if (lex_table[index]->type == OP_COMMA ||
            lex_table[index]->type == OP_SEMICOLON)
        {
            return true;
        }
        else
        {
            lexer->UngetToken(index);
            return false;
        }
    }

    ExpressionPtr ParseTableExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_LEFT_BRACE)
            THROW_PARSER_ERROR("expect '{' here");

        std::unique_ptr<TableExpression> table;
        ExpressionPtr field = ParseTableFieldExpression(lexer);
        while (field)
        {
            table->AddField(std::move(field));
            if (ParseTableFieldSeparator(lexer))
                field = ParseTableFieldExpression(lexer);
        }

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_RIGHT_BRACE)
            THROW_PARSER_ERROR("expect '}' here");
        return std::move(table);
    }

    ExpressionPtr ParseFuncCallArgsExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();
        if (index < 0)
            THROW_PARSER_ERROR("unexpect 'eof' here");

        ExpressionPtr result;
        if (lex_table[index]->type == STRING)
        {
            result = ExpressionPtr(new TermExpression(TermExpression::TERM_STRING, index));
        }
        else if (lex_table[index]->type == OP_LEFT_BRACE)
        {
            lexer->UngetToken(index);
            result = ParseTableExpression(lexer);
        }
        else if (lex_table[index]->type == OP_LEFT_PARENTHESE)
        {
            result = ParseExpListExpression(lexer);

            index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_RIGHT_PARENTHESE)
                THROW_PARSER_ERROR("expect ')' here");
        }

        return result;
    }

    ExpressionPtr ParseFuncCallExpression(
        ExpressionPtr caller, Lexer *lexer,
        bool *is_func_call = 0, int call_level = 1)
    {
        Expression *old_caller = caller.get();
        caller = ParseMemberExpression(std::move(caller), lexer);
        ExpressionPtr member = ParseFuncCallMemberExpression(lexer);
        ExpressionPtr args = ParseFuncCallArgsExpression(lexer);

        if (!args)
        {
            if (is_func_call)
                *is_func_call = (call_level != 1) && (caller.get() == old_caller);
            return caller;
        }

        caller.reset(new FuncCallExpression(std::move(caller), std::move(member), std::move(args)));
        return ParseFuncCallExpression(std::move(caller), lexer, is_func_call, call_level + 1);
    }

    ExpressionPtr ParseIdStarterExpression(Lexer *lexer, bool *is_func_call)
    {
        ExpressionPtr result = ParseNameExpression(lexer);
        result = ParseFuncCallExpression(std::move(result), lexer, is_func_call);
        return result;
    }

    ExpressionPtr ParseLeftParentheseExpression(Lexer *lexer, bool *is_func_call)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != OP_LEFT_PARENTHESE)
            THROW_PARSER_ERROR("expect '(' here");

        ExpressionPtr result = ParseBasicExpression(lexer);
        if (!result)
            THROW_PARSER_ERROR("expect expression here");

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_RIGHT_PARENTHESE)
            THROW_PARSER_ERROR("expect ')' here");

        Expression *old = result.get();
        result = ParseFuncCallExpression(std::move(result), lexer, is_func_call);
        if (result.get() == old)
            THROW_PARSER_ERROR("expect '.', '[]', 'string', 'table' or '()' here");

        return result;
    }

    ExpressionPtr ParseFuncCallOrVarExpression(Lexer *lexer, bool *is_func_call)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0)
            THROW_PARSER_ERROR("unexpect 'eof' here");

        ExpressionPtr result;
        lexer->UngetToken(index);

        switch (lex_table[index]->type)
        {
        case IDENTIFIER:
            result = ParseIdStarterExpression(lexer, is_func_call);
            break;

        case OP_LEFT_PARENTHESE:
            result = ParseLeftParentheseExpression(lexer, is_func_call);
            break;
        }

        return result;
    }

    ExpressionPtr ParseVarListExpression(ExpressionPtr var, Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        std::unique_ptr<VarListExpression> var_list(new VarListExpression);

        while (true)
        {
            var_list->AddVar(std::move(var));

            int index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_COMMA)
            {
                lexer->UngetToken(index);
                break;
            }

            bool is_func_call = false;
            var = ParseFuncCallOrVarExpression(lexer, &is_func_call);
            if (is_func_call || !var)
                THROW_PARSER_ERROR("expect 'var' here");
        }

        return std::move(var_list);
    }

    ExpressionPtr ParseAssignExpression(ExpressionPtr var, Lexer *lexer)
    {
        ExpressionPtr var_list = ParseVarListExpression(std::move(var), lexer);

        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != OP_ASSIGN)
            THROW_PARSER_ERROR("expect '=' here");

        ExpressionPtr exp_list = ParseExpListExpression(lexer);
        if (!exp_list)
            THROW_PARSER_ERROR("expect expression here");

        return ExpressionPtr(new AssignExpression(std::move(var_list), std::move(exp_list)));
    }

    ExpressionPtr ParseFuncCallOrAssignExpression(Lexer *lexer)
    {
        bool is_func_call = false;
        ExpressionPtr result = ParseFuncCallOrVarExpression(lexer, &is_func_call);

        // If the result is function call expression, then it construct a
        // statement, so we return it.
        if (is_func_call)
            return result;

        // If not a function call expression, it must construct assign expression.
        if (!result)
            THROW_PARSER_ERROR("unexpect symbol here");

        result = ParseAssignExpression(std::move(result), lexer);

        return result;
    }
} // namespace lua
