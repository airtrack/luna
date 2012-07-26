#include "Statement.h"
#include "Expression.h"
#include "Error.h"
#include "Lexer.h"
#include "State.h"
#include "NameSet.h"

namespace lua
{
#define CASE_PARSE_STATEMENT(case_exp, parse_func)              \
    case case_exp:                                              \
        lexer->UngetToken(index);                               \
        return parse_func(lexer)

#define CASE_PARSE_STATEMENT_DO(case_exp, parse_func, do_func)  \
    case case_exp:                                              \
    {                                                           \
        lexer->UngetToken(index);                               \
        auto stmt = parse_func(lexer);                          \
        do_func(lexer, stmt);                                   \
        return std::move(stmt);                                 \
    }

#define DEFAULT_PARSE_STATEMENT(parse_func)                     \
    default:                                                    \
        lexer->UngetToken(index);                               \
        return parse_func(lexer)

    inline void ResetExpResult(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_ResetCounter;
    }

    inline void CleanExpResult(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;
    }

    inline void GetNameValue(CodeWriter *writer, ExpressionPtr& name)
    {
        name->GenerateCode(writer);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_GetTableValue;
    }

    inline void GenerateBlock(CodeWriter *writer, StatementPtr& block_stmt)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_AddLocalTable;

        block_stmt->GenerateCode(writer);

        ins = writer->NewInstruction();
        ins->op_code = OpCode_DelLocalTable;
    }

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
        CASE_PARSE_STATEMENT(KW_LOCAL, ParseLocalStatement);
        CASE_PARSE_STATEMENT(KW_BREAK, ParseBreakStatement);
        CASE_PARSE_STATEMENT_DO(KW_FUNCTION, ParseFunctionStatement, CollectUpvalueFromFunc);
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

    NormalStatement::NormalStatement(ExpressionPtr &&exp)
        : exp_(std::move(exp))
    {
    }

    void NormalStatement::GenerateCode(CodeWriter *writer)
    {
        exp_->GenerateCode(writer);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;
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

    void DoStatement::GenerateCode(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_AddLocalTable;

        block_stmt_->GenerateCode(writer);

        ins = writer->NewInstruction();
        ins->op_code = OpCode_DelLocalTable;
    }

    StatementPtr ParseDoBlockEnd(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        // Parse "do"
        if (index < 0 || lex_table[index]->type != KW_DO)
            THROW_PARSER_ERROR("expect 'do' here");

        NameSetLevelPusher level_pusher(lexer->GetLocalNameSet());

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

    void WhileStatement::GenerateCode(CodeWriter *writer)
    {
        assert(writer->GetInstructionCount() > 0);
        std::size_t begin_index = writer->GetInstructionCount() - 1;

        exp_->GenerateCode(writer);
        ResetExpResult(writer);

        std::size_t jmp_false_index = writer->StartJmpInstruction(OpCode_JmpFalse);
        CleanExpResult(writer);

        GenerateBlock(writer, block_stmt_);
        writer->NewJmpInstruction(OpCode_Jmp, begin_index);

        writer->CompleteJmpInstruction(jmp_false_index);
        CleanExpResult(writer);
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

    void RepeatStatement::GenerateCode(CodeWriter *writer)
    {
        // We add a empty counter as last exp result at first time
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Push;
        ins->param_a.type = InstructionParamType_Counter;
        ins->param_a.param.counter = 0;

        std::size_t begin_index = writer->GetInstructionCount() - 1;

        // Clean last exp result
        CleanExpResult(writer);

        ins = writer->NewInstruction();
        ins->op_code = OpCode_AddLocalTable;

        block_stmt_->GenerateCode(writer);

        exp_->GenerateCode(writer);
        ResetExpResult(writer);

        ins = writer->NewInstruction();
        ins->op_code = OpCode_DelLocalTable;

        writer->NewJmpInstruction(OpCode_JmpFalse, begin_index);

        // Clean exp result when exp is true
        CleanExpResult(writer);
    }

    StatementPtr ParseRepeatStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_REPEAT)
            THROW_PARSER_ERROR("expect 'repeat' here");

        // Block statements in "repeat-until" and expression after "until"
        // has same name set level.
        NameSetLevelPusher level_pusher(lexer->GetLocalNameSet());

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
        if (!exp)
            THROW_PARSER_ERROR("expect 'exp' here");

        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_THEN)
            THROW_PARSER_ERROR("expect 'then' here");

        {
            NameSetLevelPusher level_pusher(lexer->GetLocalNameSet());
            block_stmt = ParseBlockStatement(lexer);
        }

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

    void IfStatement::GenerateCode(CodeWriter *writer)
    {
        exp_->GenerateCode(writer);
        ResetExpResult(writer);

        // If exp is false, we jump to false block statments
        std::size_t jmp_false_index = writer->StartJmpInstruction(OpCode_JmpFalse);

        CleanExpResult(writer);
        GenerateTrueBlock(writer);

        // After execute true block statements, we jump to the end of if-statement
        std::size_t jmp_end_index = writer->StartJmpInstruction(OpCode_Jmp);

        // Fill instruction index for jmp_false destination
        writer->CompleteJmpInstruction(jmp_false_index);

        CleanExpResult(writer);
        GenerateFalseBlock(writer);

        // Fill instruction index for jmp_end destination
        writer->CompleteJmpInstruction(jmp_end_index);
    }

    void IfStatement::GenerateTrueBlock(CodeWriter *writer)
    {
        GenerateBlock(writer, block_stmt_);
    }

    void IfStatement::GenerateFalseBlock(CodeWriter *writer)
    {
        if (else_stmt_)
            else_stmt_->GenerateCode(writer);
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

        return StatementPtr(new IfStatement(std::move(exp), std::move(block_stmt), std::move(else_stmt)));
    }

    ElseStatement::ElseStatement(StatementPtr &&stmt)
        : block_stmt_(std::move(stmt))
    {
    }

    void ElseStatement::GenerateCode(CodeWriter *writer)
    {
        GenerateBlock(writer, block_stmt_);
    }

    StatementPtr ParseElseStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_ELSE)
            THROW_PARSER_ERROR("expect 'else' here");

        NameSetLevelPusher level_pusher(lexer->GetLocalNameSet());
        StatementPtr stmt = ParseBlockStatement(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");

        return StatementPtr(new ElseStatement(std::move(stmt)));
    }

    NumericForStatement::NumericForStatement(ExpressionPtr &&var,
                                             ExpressionPtr &&exp1,
                                             ExpressionPtr &&limit,
                                             ExpressionPtr &&exp2,
                                             ExpressionPtr &&step,
                                             ExpressionPtr &&exp3,
                                             ExpressionPtr &&zero,
                                             StatementPtr &&block_stmt)
        : var_(std::move(var)),
          exp1_(std::move(exp1)),
          limit_(std::move(limit)),
          exp2_(std::move(exp2)),
          step_(std::move(step)),
          exp3_(std::move(exp3)),
          zero_(std::move(zero)),
          block_stmt_(std::move(block_stmt))
    {
    }

    void NumericForStatement::GenerateCode(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_AddLocalTable;

        GenerateInit(writer);
        std::size_t begin_index = writer->GetInstructionCount() - 1;

        GenerateCompareZero(writer);
        std::size_t jmp_step_greater_zero_index = writer->StartJmpInstruction(OpCode_JmpTrue);

        CleanExpResult(writer);
        GenerateCompare(writer, var_, limit_, OpCode_GreaterEqual);
        std::size_t jmp_to_block_index1 = writer->StartJmpInstruction(OpCode_JmpTrue);

        CleanExpResult(writer);
        std::size_t jmp_to_end_index1 = writer->StartJmpInstruction(OpCode_Jmp);

        writer->CompleteJmpInstruction(jmp_step_greater_zero_index);

        CleanExpResult(writer);
        GenerateCompare(writer, var_, limit_, OpCode_LessEqual);
        std::size_t jmp_to_block_index2 = writer->StartJmpInstruction(OpCode_JmpTrue);

        CleanExpResult(writer);
        std::size_t jmp_to_end_index2 = writer->StartJmpInstruction(OpCode_Jmp);
        writer->CompleteJmpInstruction(jmp_to_block_index1);
        writer->CompleteJmpInstruction(jmp_to_block_index2);

        CleanExpResult(writer);
        GenerateBody(writer);
        writer->NewJmpInstruction(OpCode_Jmp, begin_index);
        writer->CompleteJmpInstruction(jmp_to_end_index1);
        writer->CompleteJmpInstruction(jmp_to_end_index2);

        ins = writer->NewInstruction();
        ins->op_code = OpCode_DelLocalTable;
    }

    void NumericForStatement::GenerateInit(CodeWriter *writer)
    {
        GenerateAssign(writer, exp1_, var_);
        GenerateAssign(writer, exp2_, limit_);
        GenerateAssign(writer, exp3_, step_);
    }

    void NumericForStatement::GenerateAssign(CodeWriter *writer,
                                             ExpressionPtr& exp,
                                             ExpressionPtr& name)
    {
        exp->GenerateCode(writer);
        name->GenerateCode(writer);

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Assign;

        CleanExpResult(writer);
    }

    void NumericForStatement::GenerateCompare(CodeWriter *writer,
                                              ExpressionPtr& name1,
                                              ExpressionPtr& name2,
                                              OpCode op_code)
    {
        GetNameValue(writer, name1);
        GetNameValue(writer, name2);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = op_code;
    }

    void NumericForStatement::GenerateCompareZero(CodeWriter *writer)
    {
        GetNameValue(writer, step_);
        zero_->GenerateCode(writer);

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Greater;
    }

    void NumericForStatement::GenerateBody(CodeWriter *writer)
    {
        GenerateBlock(writer, block_stmt_);

        GetNameValue(writer, var_);
        GetNameValue(writer, step_);

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Plus;

        var_->GenerateCode(writer);
        ins = writer->NewInstruction();
        ins->op_code = OpCode_Assign;

        CleanExpResult(writer);
    }

    StatementPtr ParseNumericForStatement(Lexer *lexer,
                                          std::unique_ptr<NameListExpression> &&name_list,
                                          std::unique_ptr<ExpListExpression> &&exp_list)
    {
        std::size_t exp_count = exp_list->GetCount();
        if (exp_count != 2 && exp_count != 3)
            THROW_PARSER_ERROR("expect two or three expressions here");

        DataPool *data_pool = lexer->GetState()->GetDataPool();
        ExpressionPtr exp3;
        if (exp_count == 3)
            exp3 = exp_list->PopExp();
        else
            exp3.reset(new TermExpression(TermExpression::TERM_NUMBER,
                                          data_pool->GetNumber(1)));

        ExpressionPtr exp2 = exp_list->PopExp();
        ExpressionPtr exp1 = exp_list->PopExp();
        ExpressionPtr var = name_list->PopName();
        ExpressionPtr limit(new NameExpression(data_pool->GetString("@limit@"),
                                               ParseNameType_DefineLocalName));
        ExpressionPtr step(new NameExpression(data_pool->GetString("@step@"),
                                              ParseNameType_DefineLocalName));
        ExpressionPtr zero(new TermExpression(TermExpression::TERM_NUMBER,
                                              data_pool->GetNumber(0)));

        StatementPtr block_stmt = ParseDoBlockEnd(lexer);

        return StatementPtr(new NumericForStatement(
            std::move(var), std::move(exp1), std::move(limit), std::move(exp2),
            std::move(step), std::move(exp3), std::move(zero), std::move(block_stmt)));
    }

    GenericForStatement::GenericForStatement(ExpressionPtr &&f,
                                             ExpressionPtr &&s,
                                             ExpressionPtr &&var,
                                             ExpressionPtr &&name_list,
                                             ExpressionPtr &&exp_list,
                                             StatementPtr &&block_stmt)
        : f_(std::move(f)),
          s_(std::move(s)),
          var_(std::move(var)),
          name_list_(std::move(name_list)),
          exp_list_(std::move(exp_list)),
          block_stmt_(std::move(block_stmt))
    {
    }

    void GenericForStatement::GenerateCode(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_AddLocalTable;

        GenerateInit(writer);

        std::size_t begin_index = writer->GetInstructionCount() - 1;
        GenerateCall(writer);
        name_list_->GenerateCode(writer);

        GenerateAssignVar(writer);

        GetNameValue(writer, var_);
        std::size_t jmp_end_index = writer->StartJmpInstruction(OpCode_JmpNil);

        CleanExpResult(writer);
        GenerateBlock(writer, block_stmt_);
        writer->NewJmpInstruction(OpCode_Jmp, begin_index);

        writer->CompleteJmpInstruction(jmp_end_index);
        CleanExpResult(writer);

        ins = writer->NewInstruction();
        ins->op_code = OpCode_DelLocalTable;
    }

    void GenericForStatement::GenerateInit(CodeWriter *writer)
    {
        exp_list_->GenerateCode(writer);
        f_->GenerateCode(writer);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Assign;

        s_->GenerateCode(writer);
        ins = writer->NewInstruction();
        ins->op_code = OpCode_Assign;

        var_->GenerateCode(writer);
        ins = writer->NewInstruction();
        ins->op_code = OpCode_Assign;

        CleanExpResult(writer);
    }

    void GenericForStatement::GenerateCall(CodeWriter *writer)
    {
        GetNameValue(writer, f_);
        GetNameValue(writer, s_);
        GetNameValue(writer, var_);
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_MergeCounter;

        ins = writer->NewInstruction();
        ins->op_code = OpCode_Call;
    }

    void GenericForStatement::GenerateAssignVar(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_ResetCounter;

        var_->GenerateCode(writer);

        ins = writer->NewInstruction();
        ins->op_code = OpCode_Assign;

        ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;
    }

    StatementPtr ParseGenericForStatement(Lexer *lexer,
                                          ExpressionPtr &&name_list,
                                          ExpressionPtr &&exp_list)
    {
        StatementPtr block_stmt = ParseDoBlockEnd(lexer);

        DataPool *data_pool = lexer->GetState()->GetDataPool();
        ExpressionPtr f(new NameExpression(data_pool->GetString("@f@"),
                                           ParseNameType_DefineLocalName));
        ExpressionPtr s(new NameExpression(data_pool->GetString("@s@"),
                                           ParseNameType_DefineLocalName));
        ExpressionPtr var(new NameExpression(data_pool->GetString("@var@"),
                                             ParseNameType_DefineLocalName));

        return StatementPtr(new GenericForStatement(std::move(f),
                                                    std::move(s),
                                                    std::move(var),
                                                    std::move(name_list),
                                                    std::move(exp_list),
                                                    std::move(block_stmt)));
    }

    StatementPtr ParseForStatement(Lexer *lexer)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_FOR)
            THROW_PARSER_ERROR("expect 'for' here");

        NameSetLevelPusher level_pusher(lexer->GetLocalNameSet());
        std::unique_ptr<NameListExpression> name_list = ParseNameListExpression(lexer);

        index = lexer->GetToken();
        if (index < 0 || (lex_table[index]->type != OP_ASSIGN && lex_table[index]->type != KW_IN))
            THROW_PARSER_ERROR("expect '=' or 'in' here");

        std::size_t name_count = name_list->GetCount();
        if (name_count > 1 && lex_table[index]->type != KW_IN)
            THROW_PARSER_ERROR("expect 'in' here");

        std::unique_ptr<ExpListExpression> exp_list = ParseExpListExpression(lexer);
        if (!exp_list)
            THROW_PARSER_ERROR("expect expression here");

        if (lex_table[index]->type == OP_ASSIGN)
            return ParseNumericForStatement(lexer, std::move(name_list), std::move(exp_list));
        else
            return ParseGenericForStatement(lexer, std::move(name_list), std::move(exp_list));
    }

    FunctionStatement::FunctionStatement(FuncNameType name_type,
                                         String *self_name,
                                         ExpressionPtr &&func_name,
                                         ExpressionPtr &&param_list,
                                         StatementPtr &&block_stmt,
                                         Function *func)
        : name_type_(name_type),
          self_name_(self_name),
          func_name_(std::move(func_name)),
          param_list_(std::move(param_list)),
          block_stmt_(std::move(block_stmt)),
          func_(func)
    {
    }

    Function * FunctionStatement::GetFunction() const
    {
        return func_;
    }

    void FunctionStatement::GenerateCode(CodeWriter *writer)
    {
        CodeWriter func_writer(func_);
        GenerateFunctionCode(&func_writer);

        GenerateClosure(writer);
        GenerateFuncName(writer);
    }

    void FunctionStatement::GenerateFunctionCode(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_AddLocalTable;

        GenerateSelfParam(writer);

        if (param_list_)
            param_list_->GenerateCode(writer);

        // Clear params which are caller passed to the stack.
        ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;
        // Clear caller.
        ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;

        block_stmt_->GenerateCode(writer);

        // If there has no return statement in block statements, we push a
        // zero counter on stack as return value.
        ins = writer->NewInstruction();
        ins->op_code = OpCode_Push;
        ins->param_a.type = InstructionParamType_Counter;
        ins->param_a.param.counter = 0;

        ins = writer->NewInstruction();
        ins->op_code = OpCode_Ret;
    }

    void FunctionStatement::GenerateSelfParam(CodeWriter *writer)
    {
        if (name_type_ == NORMAL_FUNC_NAME_WITH_SELF)
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_GetLocalTable;

            ins = writer->NewInstruction();
            ins->op_code = OpCode_Push;
            ins->param_a.type = InstructionParamType_Name;
            ins->param_a.param.name = self_name_;

            ins = writer->NewInstruction();
            ins->op_code = OpCode_Assign;
        }
    }

    void FunctionStatement::GenerateClosure(CodeWriter *writer)
    {
        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_GenerateClosure;
        ins->param_a.type = InstructionParamType_Value;
        ins->param_a.param.value = func_;
    }

    void FunctionStatement::GenerateFuncName(CodeWriter *writer)
    {
        if (func_name_)
        {
            func_name_->GenerateCode(writer);

            // Assign closure to func name.
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_Assign;

            // Clear stack closure.
            ins = writer->NewInstruction();
            ins->op_code = OpCode_CleanStack;
        }
    }

    ExpressionPtr ParseFunctionName(Lexer *lexer, FuncNameType& type)
    {
        switch (type)
        {
        case NORMAL_FUNC_NAME:
            return ParseFuncNameExpression(lexer, type);
        case LOCAL_FUNC_NAME:
            return ParseNameExpression(lexer, ParseNameType_DefineLocalName);
        case NO_FUNC_NAME:
            // No func name, so we don't parse func name.
            break;
        }

        return ExpressionPtr();
    }

    void ParseFunctionBody(Lexer *lexer, ExpressionPtr& param_list, StatementPtr& block_stmt)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != OP_LEFT_PARENTHESE)
            THROW_PARSER_ERROR("expect '(' here");

        param_list = ParseParamListExpression(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != OP_RIGHT_PARENTHESE)
            THROW_PARSER_ERROR("expect ')' here");

        block_stmt = ParseBlockStatement(lexer);

        index = lexer->GetToken();
        if (index < 0 || lex_table[index]->type != KW_END)
            THROW_PARSER_ERROR("expect 'end' here");
    }

    std::unique_ptr<FunctionStatement> ParseFunctionStatement(Lexer *lexer, FuncNameType type)
    {
        LexTable &lex_table = lexer->GetLexTable();
        int index = lexer->GetToken();

        if (index < 0 || lex_table[index]->type != KW_FUNCTION)
            THROW_PARSER_ERROR("expect 'function' here");

        ExpressionPtr func_name = ParseFunctionName(lexer, type);

        // New up value set for each function
        std::unique_ptr<UpvalueNameSet> upvalue_set(new UpvalueNameSet);
        UpvalueNameSetter upvalue_setter(lexer, upvalue_set.get());

        // Push first local name level of function
        NameSet *name_set = lexer->GetLocalNameSet();
        NameSetLevelPusher level_pusher(name_set);
        FuncStartLevelSetter level_setter(lexer, name_set->GetLevelCount());

        String *self_name = type == NORMAL_FUNC_NAME_WITH_SELF
            ? lexer->GetState()->GetDataPool()->GetString("self") : 0;
        if (self_name)
            lexer->GetLocalNameSet()->Insert(self_name);

        ExpressionPtr param_list;
        StatementPtr block_stmt;
        ParseFunctionBody(lexer, param_list, block_stmt);

        Function *func = lexer->GetState()->GetDataPool()->GetFunction(std::move(upvalue_set));
        return std::unique_ptr<FunctionStatement>(new FunctionStatement(
            type, self_name, std::move(func_name),
            std::move(param_list), std::move(block_stmt), func));
    }

    void CollectUpvalueFromFunc(Lexer *lexer, const std::unique_ptr<FunctionStatement>& func_stmt)
    {
        Function *func = func_stmt->GetFunction();
        const UpvalueNameSet *func_upvalue_set = func->GetUpvalueSet();

        for (auto it = func_upvalue_set->Begin(); it != func_upvalue_set->End(); ++it)
        {
            // If the level of func's up value less than current function start level,
            // then this up value is current function's up value also.
            if (it->second < lexer->GetFuncStartLevel())
                lexer->GetUpvalueNameSet()->Insert(it->first, it->second);
        }
    }

    LocalStatement::LocalStatement(ExpressionPtr &&name_list,
                                   ExpressionPtr &&exp_list)
        : name_list_(std::move(name_list)),
          exp_list_(std::move(exp_list))
    {
    }

    void LocalStatement::GenerateCode(CodeWriter *writer)
    {
        if (exp_list_)
        {
            exp_list_->GenerateCode(writer);
        }
        else
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_Push;
            ins->param_a.type = InstructionParamType_Counter;
            ins->param_a.param.counter = 0;
        }

        name_list_->GenerateCode(writer);

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_CleanStack;
    }

    StatementPtr ParseLocalFunctionStatement(Lexer *lexer)
    {
        std::unique_ptr<FunctionStatement> func_stmt = ParseFunctionStatement(lexer, LOCAL_FUNC_NAME);
        CollectUpvalueFromFunc(lexer, func_stmt);
        return std::move(func_stmt);
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

    void ReturnStatement::GenerateCode(CodeWriter *writer)
    {
        if (return_exp_list_)
        {
            return_exp_list_->GenerateCode(writer);
        }
        else
        {
            Instruction *ins = writer->NewInstruction();
            ins->op_code = OpCode_Push;
            ins->param_a.type = InstructionParamType_Counter;
            ins->param_a.param.counter = 0;
        }

        Instruction *ins = writer->NewInstruction();
        ins->op_code = OpCode_Ret;
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

    StatementPtr ParseChunkStatement(Lexer *lexer)
    {
        NameSet *name_set = lexer->GetLocalNameSet();
        NameSetLevelPusher level_pusher(name_set);
        FuncStartLevelSetter level_setter(lexer, name_set->GetLevelCount());

        StatementPtr block_stmt = ParseBlockStatement(lexer);

        int index = lexer->GetToken();
        if (index != -1)
            THROW_PARSER_ERROR("expect 'eof' here");

        return std::move(block_stmt);
    }
} // namespace lua
