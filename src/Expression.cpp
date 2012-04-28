#include "Expression.h"
#include "Statement.h"
#include "Error.h"
#include "Lexer.h"
#include "State.h"
#include "NameSet.h"
#include <assert.h>
#include <stdlib.h>

namespace lua
{
#define GET_STRING_FROM_POOL(index)                 \
    lexer->GetState()->GetDataPool()->GetString(lex_table[index]->value)
#define GET_NUMBER_FROM_POOL(index)                 \
    lexer->GetState()->GetDataPool()->GetNumber(strtod(lex_table[index]->value.c_str(), 0))
#define GET_NIL_FROM_POOL()                         \
    lexer->GetState()->GetDataPool()->GetNil()
#define GET_TRUE_FROM_POOL()                        \
    lexer->GetState()->GetDataPool()->GetTrue()
#define GET_FALSE_FROM_POOL()                       \
    lexer->GetState()->GetDataPool()->GetFalse()

    TermExpression::TermExpression(TermType type, Value *value)
        : type_(type),
          value_(value)
    {
    }

    void TermExpression::GenerateCode(CodeWriter *writer)
    {
        if (type_ == TERM_PARAM_LIST)
        {
            // TODO: generate code for param list
        }
        else
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_Push;
            ins->param_a.type = InstructionParamType_Value;
            ins->param_a.param.value = value_;

            ins = writer->NewInstruction();
            ins->op_code = OpCode_Push;
            ins->param_a.type = InstructionParamType_Counter;
            ins->param_a.param.counter = 1;
        }
    }

    BinaryExpression::BinaryExpression(BinaryType type, ExpressionPtr &&left_exp, ExpressionPtr &&right_exp)
        : type_(type),
          left_exp_(std::move(left_exp)),
          right_exp_(std::move(right_exp))
    {
    }

    void BinaryExpression::GenerateCode(CodeWriter *writer)
    {
        if (type_ == BINARY_TYPE_AND)
            GenerateAnd(writer);
        else if (type_ == BINARY_TYPE_OR)
            GenerateOr(writer);
        else
            GenerateOther(writer);
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

    void BinaryExpression::GenerateAnd(CodeWriter *writer)
    {
        GenerateJmp(writer, OpCode_JmpFalse);
    }

    void BinaryExpression::GenerateOr(CodeWriter *writer)
    {
        GenerateJmp(writer, OpCode_JmpTrue);
    }

    void BinaryExpression::GenerateJmp(CodeWriter *writer, OpCode op_code)
    {
        GenerateLeftExp(writer);

        Instruction *jmp = writer->NewInstruction();
        jmp->op_code = op_code;
        jmp->param_a.type = InstructionParamType_OpCodeIndex;
        std::size_t jmp_index = writer->GetInstructionCount() - 1;

        // If not execute jmp instruction then clean the left exp result,
        // because "exp1 and exp2" return "exp2" result when "exp1" is true,
        // and "exp1 or exp2" return "exp2" result when "exp1" is false.
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;

        GenerateRightExp(writer);

        // Fill opcode index for jmp instruction destination.
        jmp = writer->GetInstruction(jmp_index);
        jmp->param_a.param.opcode_index = writer->GetInstructionCount() - 1;
    }

    void BinaryExpression::GenerateOther(CodeWriter *writer)
    {
        GenerateLeftExp(writer);
        GenerateRightExp(writer);
        GenerateOpInstruction(writer);
    }

    void BinaryExpression::GenerateLeftExp(CodeWriter *writer)
    {
        left_exp_->GenerateCode(writer);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_ResetCounter;
    }

    void BinaryExpression::GenerateRightExp(CodeWriter *writer)
    {
        right_exp_->GenerateCode(writer);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_ResetCounter;
    }

#define CASE_OPERATOR_OPCODE(op, opcode)        \
    case op:                                    \
        ins->op_code = opcode;                  \
        break

    void BinaryExpression::GenerateOpInstruction(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        switch (type_)
        {
        CASE_OPERATOR_OPCODE(BINARY_TYPE_POWER, OpCode_Power);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_MULTIPLY, OpCode_Multiply);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_DIVIDE, OpCode_Divide);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_MOD, OpCode_Mod);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_PLUS, OpCode_Plus);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_MINUS, OpCode_Minus);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_CONCAT, OpCode_Concat);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_LESS, OpCode_Less);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_GREATER, OpCode_Greater);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_LESS_EQUAL, OpCode_LessEqual);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_GREATER_EQUAL, OpCode_GreaterEqual);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_NOT_EQUAL, OpCode_NotEqual);
        CASE_OPERATOR_OPCODE(BINARY_TYPE_EQUAL, OpCode_Equal);
        }
    }

    UnaryExpression::UnaryExpression(UnaryType type, ExpressionPtr &&exp)
        : type_(type),
          exp_(std::move(exp))
    {
    }

    void UnaryExpression::GenerateCode(CodeWriter *writer)
    {
        exp_->GenerateCode(writer);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_ResetCounter;

        ins = writer->NewInstruction();
        switch (type_)
        {
        CASE_OPERATOR_OPCODE(UNARY_TYPE_NOT, OpCode_Not);
        CASE_OPERATOR_OPCODE(UNARY_TYPE_LENGTH, OpCode_Length);
        CASE_OPERATOR_OPCODE(UNARY_TYPE_NEGATIVE, OpCode_Negative);
        }
    }

    TableKeyExpression::TableKeyExpression(TableKeyType type, ExpressionPtr &&key)
        : type_(type),
          key_(std::move(key))
    {
    }

    void TableKeyExpression::GenerateCode(CodeWriter *writer)
    {
        key_->GenerateCode(writer);

        if (type_ == TableKeyType_Exp)
        {
            // Reset counter as 1
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_ResetCounter;

            // Pop counter
            ins = writer->NewInstruction();
            ins->op_code = OpCode_Pop;
        }
    }

    TableExpression::TableExpression()
        : array_index_helper_(0)
    {
    }

    void TableExpression::GenerateCode(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_NewTable;

        if (fields_.empty())
            return ;

        auto it1 = fields_.begin();
        auto it2 = --fields_.end();
        for (; it1 != it2; ++it1)
            GenerateField(writer, it1, true);

        // for last field, we do not reset value counter
        GenerateField(writer, it2, false);
    }

    void TableExpression::GenerateField(CodeWriter *writer, Fields::iterator it, bool reset_value)
    {
        it->second->GenerateCode(writer);
        Instruction *ins = 0;

        if (reset_value)
        {
            ins = writer->NewInstruction();
            ins->op_code = OpCode_ResetCounter;
        }

        ins = writer->NewInstruction();
        ins->op_code = OpCode_DuplicateCounter;
        ins->param_a.type = InstructionParamType_CounterIndex;
        ins->param_a.param.counter_index = 1;

        if (it->first)
        {
            it->first->GenerateCode(writer);
            ins = writer->NewInstruction();
            ins->op_code = OpCode_Assign;
        }
        else
        {
            ins = writer->NewInstruction();
            ins->op_code = OpCode_SetTableArrayValue;
            ins->param_a.type = InstructionParamType_ArrayIndex;
            ins->param_a.param.array_index = ++array_index_helper_;
        }

        ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;
    }

    MemberExpression::MemberExpression(ExpressionPtr &&table,
                                       ExpressionPtr &&member,
                                       MemberType member_type)
        : table_exp_(std::move(table)),
          member_exp_(std::move(member)),
          member_type_(member_type)
    {
    }

    void MemberExpression::GenerateCode(CodeWriter *writer)
    {
        table_exp_->GenerateCode(writer);

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_GetTableValue;

        member_exp_->GenerateCode(writer);

        if (member_type_ == MemberType_Sub)
        {
            // Reset counter as 1
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_ResetCounter;

            // Pop the counter
            ins = writer->NewInstruction();
            ins->op_code = OpCode_Pop;
        }
    }

    NameExpression::NameExpression(String *name, ParseNameType parse_name_type)
        : name_(name),
          parse_name_type_(parse_name_type)
    {
    }

    void NameExpression::GenerateCode(CodeWriter *writer)
    {
        GenerateNameTypeCode(writer);

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Push;
        ins->param_a.type = InstructionParamType_Name;
        ins->param_a.param.name = name_;
    }

    void NameExpression::GenerateNameTypeCode(CodeWriter *writer)
    {
        Instruction *ins = 0;
        switch (parse_name_type_)
        {
        case ParseNameType_DefineLocalName:
            ins = writer->NewInstruction();
            ins->op_code = OpCode_GetLocalTable;
            break;
        case ParseNameType_GetName:
            ins = writer->NewInstruction();
            ins->op_code = OpCode_GetTable;
            ins->param_a.type = InstructionParamType_Name;
            ins->param_a.param.name = name_;
            break;
        case ParseNameType_GetMemberName:
            // Do nothing.
            break;
        }
    }

    std::size_t NameListExpression::GetCount() const
    {
        return name_list_.size();
    }

    void NameListExpression::AddName(ExpressionPtr &&name)
    {
        name_list_.push_back(std::move(name));
    }

    ExpressionPtr NameListExpression::PopName()
    {
        if (name_list_.empty())
            return ExpressionPtr();

        ExpressionPtr result = std::move(name_list_.back());
        name_list_.pop_back();
        return std::move(result);
    }

    void NameListExpression::GenerateCode(CodeWriter *writer)
    {
        for (auto it = name_list_.begin(); it != name_list_.end(); ++it)
        {
            (*it)->GenerateCode(writer);

            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_Assign;
        }
    }

    std::size_t ExpListExpression::GetCount() const
    {
        return exp_list_.size();
    }

    void ExpListExpression::AddExp(ExpressionPtr &&exp)
    {
        exp_list_.push_back(std::move(exp));
    }

    ExpressionPtr ExpListExpression::PopExp()
    {
        if (exp_list_.empty())
            return ExpressionPtr();

        ExpressionPtr result = std::move(exp_list_.back());
        exp_list_.pop_back();
        return std::move(result);
    }

    void ExpListExpression::GenerateCode(CodeWriter *writer)
    {
        if (exp_list_.empty())
            PushEmptyCounter(writer);
        else
            ExpListGenerateCode(writer);
    }

    void ExpListExpression::PushEmptyCounter(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Push;
        ins->param_a.type = InstructionParamType_Counter;
        ins->param_a.param.counter = 0;
    }

    void ExpListExpression::ExpListGenerateCode(CodeWriter *writer)
    {
        int counter_num = 0;
        for (auto it = exp_list_.begin(); it != exp_list_.end(); ++it)
        {
            if (counter_num > 1)
            {
                Instruction *ins = writer->NewInstruction();
                ins->op_code = OpCode_MergeCounter;
            }

            (*it)->GenerateCode(writer);
            ++counter_num;
        }

        if (counter_num > 1)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_MergeCounter;
        }
    }

    std::size_t VarListExpression::GetCount() const
    {
        return var_list_.size();
    }

    void VarListExpression::AddVar(ExpressionPtr &&var)
    {
        var_list_.push_back(std::move(var));
    }

    void VarListExpression::GenerateCode(CodeWriter *writer)
    {
        for (auto it = var_list_.begin(); it != var_list_.end(); ++it)
        {
            (*it)->GenerateCode(writer);
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_Assign;
        }
    }

    FuncNameExpression::FuncNameExpression(ExpressionPtr &&pre_name, ExpressionPtr &&member)
        : pre_name_(std::move(pre_name)),
          member_(std::move(member))
    {
    }

    void FuncNameExpression::GenerateCode(CodeWriter *writer)
    {
        pre_name_->GenerateCode(writer);

        if (member_)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_GetTableValue;

            member_->GenerateCode(writer);
        }
    }

    ParamListExpression::ParamListExpression(ExpressionPtr &&name_list, bool has_dot3)
        : name_list_(std::move(name_list)),
          has_dot3_(has_dot3)
    {
    }

    void ParamListExpression::GenerateCode(CodeWriter *writer)
    {
        if (name_list_)
            name_list_->GenerateCode(writer);

        if (has_dot3_)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_GenerateArgTable;
        }
    }

    CallerExpression::CallerExpression(ExpressionPtr &&caller, bool is_variable)
        : caller_(std::move(caller)),
          is_variable_(is_variable)
    {
    }

    void CallerExpression::GenerateCode(CodeWriter *writer)
    {
        caller_->GenerateCode(writer);

        Instruction *ins = writer->NewInstruction();
        if (is_variable_)
            ins->op_code = OpCode_GetTableValue;
        else
            ins->op_code = OpCode_ResetCounter;
    }

    FuncCallExpression::FuncCallExpression(ExpressionPtr &&caller,
        ExpressionPtr &&member, ExpressionPtr &&arg_list)
        : caller_(std::move(caller)),
          member_(std::move(member)),
          arg_list_(std::move(arg_list))
    {
    }

    void FuncCallExpression::GenerateCode(CodeWriter *writer)
    {
        caller_->GenerateCode(writer);

        if (member_)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_DuplicateCounter;

            member_->GenerateCode(writer);

            ins = writer->NewInstruction();
            ins->op_code = OpCode_GetTableValue;
            ins->param_a.type = InstructionParamType_CounterIndex;
            ins->param_a.param.counter_index = 1;
        }

        arg_list_->GenerateCode(writer);

        if (member_)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_MergeCounter;
        }

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Call;
    }

    AssignExpression::AssignExpression(std::unique_ptr<VarListExpression> &&var_list,
                                       std::unique_ptr<ExpListExpression> &&exp_list)
        : var_list_(std::move(var_list)),
          exp_list_(std::move(exp_list))
    {
    }

    void AssignExpression::GenerateCode(CodeWriter *writer)
    {
        exp_list_->GenerateCode(writer);
        var_list_->GenerateCode(writer);
    }

    FuncDefineExpression::FuncDefineExpression(StatementPtr &&func_def)
        : func_def_(std::move(func_def))
    {
    }

    void FuncDefineExpression::GenerateCode(CodeWriter *writer)
    {
        func_def_->GenerateCode(writer);
    }

    PreExpExpression::PreExpExpression(ExpressionPtr &&exp, PreExpType type)
        : exp_(std::move(exp)),
          type_(type)
    {
    }

    void PreExpExpression::GenerateCode(CodeWriter *writer)
    {
        exp_->GenerateCode(writer);
        if (type_ == PreExp_Variable)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_GetTableValue;
        }
        else if (type_ == PreExp_ParentheseExp)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_ResetCounter;
        }
    }

    ExpressionPtr ParseFuncCallOrVarExpression(Lexer *lexer, PreExpType *type);
    ExpressionPtr ParseTableExpression(Lexer *lexer);

    ExpressionPtr ParseDotMemberExpression(Lexer *lexer)
    {
        ExpressionPtr member = ParseNameExpression(lexer, ParseNameType_GetMemberName);
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

        if (index < 0 || lex_table[index]->type != OP_DOT && lex_table[index]->type != OP_LEFT_BRACKET)
        {
            lexer->UngetToken(index);
            return std::move(table);
        }

        ExpressionPtr member;
        MemberType member_type;
        if (lex_table[index]->type == OP_DOT)
        {
            member_type = MemberType_Dot;
            member = ParseDotMemberExpression(lexer);
        }
        else
        {
            member_type = MemberType_Sub;
            member = ParseSubMemberExpression(lexer);
        }

        table.reset(new MemberExpression(std::move(table), std::move(member), member_type));
        return ParseMemberExpression(std::move(table), lexer);
    }

    ExpressionPtr ParseNameExpression(Lexer *lexer, ParseNameType type)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != IDENTIFIER)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        String *name = GET_STRING_FROM_POOL(index);
        if (type == ParseNameType_DefineLocalName)
            lexer->GetLocalNameSet()->Insert(name);
        else if(type == ParseNameType_GetName)
        {
            std::size_t level = lexer->GetLocalNameSet()->GetNameLevel(name);
            if (level < lexer->GetFuncStartLevel())
                lexer->GetUpvalueNameSet()->Insert(name, level);
        }

        return ExpressionPtr(new NameExpression(name, type));
    }

    std::unique_ptr<NameListExpression> ParseNameListExpression(Lexer *lexer)
    {
        ExpressionPtr name = ParseNameExpression(lexer, ParseNameType_DefineLocalName);
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

            name = ParseNameExpression(lexer, ParseNameType_DefineLocalName);
            if (!name)
                THROW_PARSER_ERROR("expect 'name' here");
        }

        return std::move(name_list);
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
        Value *value = 0;
        switch (lex_table[index]->type)
        {
        case KW_NIL:
            type = TermExpression::TERM_NIL;
            value = GET_NIL_FROM_POOL();
            break;
        case KW_FALSE:
            type = TermExpression::TERM_FALSE;
            value = GET_FALSE_FROM_POOL();
            break;
        case KW_TRUE:
            type = TermExpression::TERM_TRUE;
            value = GET_TRUE_FROM_POOL();
            break;
        case NUMBER:
            type = TermExpression::TERM_NUMBER;
            value = GET_NUMBER_FROM_POOL(index);
            break;
        case STRING:
            type = TermExpression::TERM_STRING;
            value = GET_STRING_FROM_POOL(index);
            break;
        case OP_PARAM_LIST:
            type = TermExpression::TERM_PARAM_LIST;
            break;
        default:
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        return ExpressionPtr(new TermExpression(type, value));
    }

    ExpressionPtr ParseFuncDefineExpression(Lexer *lexer)
    {
        std::unique_ptr<FunctionStatement> func_stmt = ParseFunctionStatement(lexer, NO_FUNC_NAME);
        CollectUpvalueFromFunc(lexer, func_stmt);
        return ExpressionPtr(new FuncDefineExpression(std::move(func_stmt)));
    }

    ExpressionPtr ParsePreexpExpression(Lexer *lexer)
    {
        PreExpType type;
        ExpressionPtr exp = ParseFuncCallOrVarExpression(lexer, &type);
        if (!exp)
            return std::move(exp);
        return ExpressionPtr(new PreExpExpression(std::move(exp), type));
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
            break;
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
            return std::move(exp_list);

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

        return std::move(exp_list);
    }

    ExpressionPtr ParseFuncMemberNameExpression(ExpressionPtr table, Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != OP_DOT)
        {
            lexer->UngetToken(index);
            return std::move(table);
        }

        ExpressionPtr member = ParseDotMemberExpression(lexer);
        table.reset(new MemberExpression(std::move(table), std::move(member), MemberType_Dot));

        return ParseFuncMemberNameExpression(std::move(table), lexer);
    }

    ExpressionPtr ParseFuncNameExpression(Lexer *lexer, FuncNameType& type)
    {
        ExpressionPtr name = ParseNameExpression(lexer, ParseNameType_GetName);
        if (!name)
            THROW_PARSER_ERROR("expect 'name' here");

        name = ParseFuncMemberNameExpression(std::move(name), lexer);

        ExpressionPtr member;
        LexTable &lex_table = lexer->GetLexTable();

        int index = lexer->GetToken();
        if (index >= 0 && lex_table[index]->type == OP_COLON)
        {
            member = ParseNameExpression(lexer, ParseNameType_GetMemberName);
            if (!member)
                THROW_PARSER_ERROR("expect 'name' here");
            type = NORMAL_FUNC_NAME_WITH_SELF;
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
        ExpressionPtr name = ParseNameExpression(lexer, ParseNameType_DefineLocalName);
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

            name = ParseNameExpression(lexer, ParseNameType_DefineLocalName);
            if (!name)
            {
                lexer->UngetToken(index);
                break;
            }
        }

        return std::move(name_list);
    }

    bool ParseParamDot3Expression(bool need_comma, Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();

        int index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_COMMA && lex_table[index]->type != OP_PARAM_LIST)
        {
            lexer->UngetToken(index);
            return false;
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

        // Define "arg" local name for "...".
        String *arg = lexer->GetState()->GetDataPool()->GetString("arg");
        lexer->GetLocalNameSet()->Insert(arg);
        return true;
    }

    ExpressionPtr ParseParamListExpression(Lexer *lexer)
    {
        ExpressionPtr name_list = ParseParamNameListExpression(lexer);
        bool has_dot3 = ParseParamDot3Expression(static_cast<bool>(name_list), lexer);
        ExpressionPtr param_list;

        if (name_list || has_dot3)
            param_list.reset(new ParamListExpression(std::move(name_list), has_dot3));

        return param_list;
    }

    ExpressionPtr ParseFuncCallMemberExpression(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        ExpressionPtr member;
        if (lex_table[index]->type == OP_COLON)
        {
            member = ParseNameExpression(lexer, ParseNameType_GetMemberName);
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
        if (index < 0 || lex_table[index]->type != OP_LEFT_BRACKET && lex_table[index]->type != IDENTIFIER)
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

            key.reset(new TableKeyExpression(TableKeyType_Exp, std::move(key)));
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
                key.reset(new NameExpression(GET_STRING_FROM_POOL(index), ParseNameType_GetMemberName));
                key.reset(new TableKeyExpression(TableKeyType_Name, std::move(key)));
            }
        }

        return key;
    }

    ExpressionPtr ParseTableFieldValueExpression(Lexer *lexer)
    {
        return ParseExpression(lexer);
    }

    std::pair<ExpressionPtr, ExpressionPtr> ParseTableFieldExpression(Lexer *lexer)
    {
        ExpressionPtr key = ParseTableFieldKeyExpression(lexer);
        ExpressionPtr value = ParseTableFieldValueExpression(lexer);

        if (value)
            return std::make_pair<ExpressionPtr, ExpressionPtr>(std::move(key), std::move(value));

        return std::make_pair(ExpressionPtr(), ExpressionPtr());
    }

    bool ParseTableFieldSeparator(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index >= 0 &&
            (lex_table[index]->type == OP_COMMA || lex_table[index]->type == OP_SEMICOLON))
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

        std::unique_ptr<TableExpression> table(new TableExpression);
        std::pair<ExpressionPtr, ExpressionPtr> field = ParseTableFieldExpression(lexer);
        while (field.second)
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
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        ExpressionPtr result;
        if (lex_table[index]->type == STRING)
        {
            result = ExpressionPtr(new TermExpression(
                TermExpression::TERM_STRING, GET_STRING_FROM_POOL(index)));
        }
        else if (lex_table[index]->type == OP_LEFT_BRACE)
        {
            lexer->UngetToken(index);
            result = ParseTableExpression(lexer);
        }
        else if (lex_table[index]->type == OP_LEFT_PARENTHESE)
        {
            result = ParseExpListExpression(lexer);
            if (!result)
                result.reset(new ExpListExpression);

            index = lexer->GetToken();
            if (index < 0 || lex_table[index]->type != OP_RIGHT_PARENTHESE)
                THROW_PARSER_ERROR("expect ')' here");
        }
        else
        {
            lexer->UngetToken(index);
        }

        return result;
    }

    ExpressionPtr ParseFuncCallExpression(
        ExpressionPtr caller, Lexer *lexer,
        bool *is_func_call = 0, bool caller_is_variable = false, int call_level = 1)
    {
        Expression *old_caller = caller.get();
        caller = ParseMemberExpression(std::move(caller), lexer);
        ExpressionPtr member = ParseFuncCallMemberExpression(lexer);
        ExpressionPtr args = ParseFuncCallArgsExpression(lexer);

        if (!args)
        {
            if (is_func_call)
                *is_func_call = (call_level != 1) && (caller.get() == old_caller);
            return std::move(caller);
        }

        caller.reset(new CallerExpression(std::move(caller), caller_is_variable));
        caller.reset(new FuncCallExpression(std::move(caller), std::move(member), std::move(args)));
        return ParseFuncCallExpression(std::move(caller), lexer, is_func_call, false, call_level + 1);
    }

    ExpressionPtr ParseIdStarterExpression(Lexer *lexer, PreExpType *type)
    {
        ExpressionPtr result = ParseNameExpression(lexer, ParseNameType_GetName);

        bool is_func_call = false;
        result = ParseFuncCallExpression(std::move(result), lexer, &is_func_call, true);

        if (type)
            *type = is_func_call ? PreExp_FuncCall : PreExp_Variable;
        return result;
    }

    ExpressionPtr ParseLeftParentheseExpression(Lexer *lexer, PreExpType *type)
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
        bool is_func_call = false;
        result = ParseFuncCallExpression(std::move(result), lexer, &is_func_call);

        PreExpType t;
        if (result.get() == old)
            t = PreExp_ParentheseExp;
        else
            t = is_func_call ? PreExp_FuncCall : PreExp_Variable;

        if (type)
            *type = t;
        return result;
    }

    ExpressionPtr ParseFuncCallOrVarExpression(Lexer *lexer, PreExpType *type)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0)
        {
            lexer->UngetToken(index);
            return ExpressionPtr();
        }

        ExpressionPtr result;
        lexer->UngetToken(index);

        switch (lex_table[index]->type)
        {
        case IDENTIFIER:
            result = ParseIdStarterExpression(lexer, type);
            break;

        case OP_LEFT_PARENTHESE:
            result = ParseLeftParentheseExpression(lexer, type);
            break;
        }

        return result;
    }

    std::unique_ptr<VarListExpression> ParseVarListExpression(ExpressionPtr var, Lexer *lexer)
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

            PreExpType type;
            var = ParseFuncCallOrVarExpression(lexer, &type);
            if (type != PreExp_Variable)
                THROW_PARSER_ERROR("expect 'var' here");
        }

        return std::move(var_list);
    }

    ExpressionPtr ParseAssignExpression(ExpressionPtr var, Lexer *lexer)
    {
        auto var_list = ParseVarListExpression(std::move(var), lexer);

        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != OP_ASSIGN)
            THROW_PARSER_ERROR("expect '=' here");

        auto exp_list = ParseExpListExpression(lexer);
        if (!exp_list)
            THROW_PARSER_ERROR("expect expression here");

        return ExpressionPtr(new AssignExpression(std::move(var_list), std::move(exp_list)));
    }

    ExpressionPtr ParseFuncCallOrAssignExpression(Lexer *lexer)
    {
        PreExpType type;
        ExpressionPtr result = ParseFuncCallOrVarExpression(lexer, &type);

        // If the result is function call expression, then it construct a
        // statement, so we return it.
        if (type == PreExp_FuncCall)
            return result;

        // Can not be "(exp)"
        if (type == PreExp_ParentheseExp)
            THROW_PARSER_ERROR("expect 'var' here");

        // If not a function call expression, it must construct assign expression.
        if (!result)
            THROW_PARSER_ERROR("unexpect symbol here");

        return ParseAssignExpression(std::move(result), lexer);
    }
} // namespace lua
