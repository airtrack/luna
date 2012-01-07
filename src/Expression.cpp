#include "Expression.h"
#include "Statement.h"
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

    BinaryExpression::BinaryType BinaryExpression::GetBinaryType(TokenType type)
    {
        switch (type)
        {
        case OP_POWER:
            return BINARY_TYPE_POWER;
        case OP_MULTIPLY:
            return BINARY_TYPE_MULTIPLY;
        case OP_DIVIDE:
            return BINARY_TYPE_DIVIDE;
        case OP_MOD:
            return BINARY_TYPE_MOD;
        case OP_PLUS:
            return BINARY_TYPE_PLUS;
        case OP_MINUS:
            return BINARY_TYPE_MINUS;
        case OP_CONCAT:
            return BINARY_TYPE_CONCAT;
        case OP_LESS:
            return BINARY_TYPE_LESS;
        case OP_GREATER:
            return BINARY_TYPE_GREATER;
        case OP_LESSEQUAL:
            return BINARY_TYPE_LESS_EQUAL;
        case OP_GREATEREQUAL:
            return BINARY_TYPE_GREATER_EQUAL;
        case OP_NOTEQUAL:
            return BINARY_TYPE_NOT_EQUAL;
        case OP_EQUAL:
            return BINARY_TYPE_EQUAL;
        case KW_AND:
            return BINARY_TYPE_AND;
        case KW_OR:
            return BINARY_TYPE_OR;
        default:
            return BINARY_TYPE_NONE;
        }
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

    FuncDefineExpression::FuncDefineExpression(ParseTreeNodePtr func_def)
        : func_def_(std::move(func_def))
    {
    }

    ExpressionPtr ParseFuncCallOrVarExpression(Lexer *lexer, bool *is_func_call);
    ExpressionPtr ParseTableExpression(Lexer *lexer);

    ExpressionPtr ParseDotMemberExpression(Lexer *lexer)
    {
        ExpressionPtr member = ParseNameExpression(lexer);
        if (!member)
            THROW_PARSER_ERROR("expect 'id' here");
        return member;
    }

    ExpressionPtr ParseSubMemberExpression(Lexer *lexer)
    {
        ExpressionPtr member = ParseExpression(lexer);
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

    ExpressionPtr ParseTermExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();
        if (index < 0)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        TermExpression::TermType type;
        switch (lex_table[index]->type)
        {
        case KW_NIL:
            type = TermExpression::TERM_NIL;
            break;
        case KW_FALSE:
            type = TermExpression::TERM_FALSE;
            break;
        case KW_TRUE:
            type = TermExpression::TERM_TRUE;
            break;
        case NUMBER:
            type = TermExpression::TERM_NUMBER;
            break;
        case STRING:
            type = TermExpression::TERM_STRING;
            break;
        case OP_PARAM_LIST:
            type = TermExpression::TERM_PARAM_LIST;
            break;
        default:
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        return ExpressionPtr(new TermExpression(type, index));
    }

    ExpressionPtr ParseFuncDefineExpression(Lexer *lexer)
    {
        ParseTreeNodePtr func(new FunctionStatement(FunctionStatement::NO_FUNC_NAME));
        func->ParseNode(lexer);
        return ExpressionPtr(new FuncDefineExpression(std::move(func)));
    }

    ExpressionPtr ParsePreexpExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();
        if (index < 0)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        if (lex_table[index]->type == OP_LEFT_PARENTHESE)
        {
            ExpressionPtr exp = ParseExpression(lexer);
            if (!exp)
                THROW_PARSER_ERROR("expect expression here");
            index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_RIGHT_PARENTHESE)
                THROW_PARSER_ERROR("expect ')' here");
            return exp;
        }
        else
        {
            lexer->UngetToken(index);
            ExpressionPtr exp = ParseFuncCallOrVarExpression(lexer, 0);
            return exp;
        }
    }

    ExpressionPtr ParseFactorExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();
        if (index < 0)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        ExpressionPtr exp;
        lexer->UngetToken(index);
        switch (lex_table[index]->type)
        {
        case KW_NIL:
        case KW_FALSE:
        case KW_TRUE:
        case NUMBER:
        case STRING:
        case OP_PARAM_LIST:
            exp = ParseTermExpression(lexer);
            break;
        case KW_FUNCTION:
            exp = ParseFuncDefineExpression(lexer);
            break;
        case OP_LEFT_BRACE:
            exp = ParseTableExpression(lexer);
            break;
        default:
            exp = ParsePreexpExpression(lexer);
            break;
        }

        return exp;
    }

    template<typename ParseExpFunc, typename TokenChecker>
    ExpressionPtr ParseBinaryExpression(Lexer *lexer, ParseExpFunc parse_exp_func, TokenChecker token_checker)
    {
        ExpressionPtr left_exp = parse_exp_func(lexer);
        if (!left_exp)
            return left_exp;

        LexTable &lex_table = lexer->GetLexTable();
        while (true)
        {
            int index = lexer->GetToken();
            if (index < 0 || !token_checker(lex_table[index]->type))
            {
                lexer->UngetToken(index);
                return left_exp;
            }

            ExpressionPtr right_exp = parse_exp_func(lexer);
            if (!right_exp)
                THROW_PARSER_ERROR("expect expression here");
            BinaryExpression::BinaryType type = BinaryExpression::GetBinaryType(lex_table[index]->type);
            left_exp.reset(new BinaryExpression(type, std::move(left_exp), std::move(right_exp)));
        }
    }

    ExpressionPtr ParsePowerExpression(Lexer *lexer)
    {
        return ParseBinaryExpression(lexer, ParseFactorExpression,
            [](int type) { return type == OP_POWER; });
    }

    ExpressionPtr ParseUnaryExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();
        if (index < 0)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        UnaryExpression::UnaryType unary_type;
        switch (lex_table[index]->type)
        {
        case KW_NOT:
            unary_type = UnaryExpression::UNARY_TYPE_NOT;
            break;
        case OP_POUND:
            unary_type = UnaryExpression::UNARY_TYPE_LENGTH;
            break;
        case OP_MINUS:
            unary_type = UnaryExpression::UNARY_TYPE_NEGATIVE;
        default:
            lexer->UngetToken(index);
            return ParsePowerExpression(lexer);
        }

        ExpressionPtr exp = ParseUnaryExpression(lexer);
        if (!exp)
            THROW_PARSER_ERROR("expect expression here");
        return ExpressionPtr(new UnaryExpression(unary_type, std::move(exp)));
    }

    ExpressionPtr ParseMultiplyDivideExpression(Lexer *lexer)
    {
        return ParseBinaryExpression(lexer, ParseUnaryExpression,
            [](int type) { return type == OP_MULTIPLY || type == OP_DIVIDE || type == OP_MOD; });
    }

    ExpressionPtr ParsePlusMinusExpression(Lexer *lexer)
    {
        return ParseBinaryExpression(lexer, ParseMultiplyDivideExpression,
            [](int type) { return type == OP_PLUS || type == OP_MINUS; });
    }

    ExpressionPtr ParseConcatExpression(Lexer *lexer)
    {
        return ParseBinaryExpression(lexer, ParsePlusMinusExpression,
            [](int type) { return type == OP_CONCAT; });
    }

    ExpressionPtr ParseRelationOpExpression(Lexer *lexer)
    {
        return ParseBinaryExpression(lexer, ParseConcatExpression,
            [](int type) {
                return type == OP_LESS || type == OP_GREATER || type == OP_EQUAL ||
                    type == OP_LESSEQUAL || type == OP_GREATEREQUAL || type == OP_NOTEQUAL;
        });
    }

    ExpressionPtr ParseAndExpression(Lexer *lexer)
    {
        return ParseBinaryExpression(lexer, ParseRelationOpExpression,
            [](int type) { return type == KW_AND; });
    }

    ExpressionPtr ParseOrExpression(Lexer *lexer)
    {
        return ParseBinaryExpression(lexer, ParseAndExpression,
            [](int type) { return type == KW_OR; });
    }

    ExpressionPtr ParseExpression(Lexer *lexer)
    {
        return ParseOrExpression(lexer);
    }

    std::unique_ptr<ExpListExpression> ParseExpListExpression(Lexer *lexer)
    {
        std::unique_ptr<ExpListExpression> exp_list;
        ExpressionPtr exp = ParseExpression(lexer);
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

            exp = ParseExpression(lexer);
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
            key = ParseExpression(lexer);
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
        return ParseExpression(lexer);
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

        ExpressionPtr result = ParseExpression(lexer);
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
