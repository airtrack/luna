#include "SemanticAnalysis.h"
#include "Visitor.h"
#include "Exception.h"
#include "String.h"
#include <string>
#include <set>
#include <assert.h>

namespace luna
{
    // Lexical block data in LexicalFunction for name finding
    struct LexicalBlock
    {
        LexicalBlock *parent_;
        std::set<std::string> names_;

        LexicalBlock() : parent_(nullptr) { }
    };

    // Lexical function data for name finding
    struct LexicalFunction
    {
        LexicalFunction *parent_;
        LexicalBlock *current_block_;
        const SyntaxTree *current_loop_;

        LexicalFunction()
            : parent_(nullptr), current_block_(nullptr), current_loop_(nullptr) { }
    };

    class SemanticAnalysisVisitor : public Visitor
    {
    public:
        virtual void Visit(Chunk *, void *);
        virtual void Visit(Block *, void *);
        virtual void Visit(ReturnStatement *, void *);
        virtual void Visit(BreakStatement *, void *);
        virtual void Visit(DoStatement *, void *);
        virtual void Visit(WhileStatement *, void *);
        virtual void Visit(RepeatStatement *, void *);
        virtual void Visit(IfStatement *, void *);
        virtual void Visit(ElseIfStatement *, void *);
        virtual void Visit(ElseStatement *, void *);
        virtual void Visit(NumericForStatement *, void *);
        virtual void Visit(GenericForStatement *, void *);
        virtual void Visit(FunctionStatement *, void *);
        virtual void Visit(FunctionName *, void *);
        virtual void Visit(LocalFunctionStatement *, void *);
        virtual void Visit(LocalNameListStatement *, void *);
        virtual void Visit(AssignmentStatement *, void *);
        virtual void Visit(VarList *, void *);
        virtual void Visit(Terminator *, void *);
        virtual void Visit(BinaryExpression *, void *);
        virtual void Visit(UnaryExpression *, void *);
        virtual void Visit(FunctionBody *, void *);
        virtual void Visit(ParamList *, void *);
        virtual void Visit(NameList *, void *);
        virtual void Visit(TableDefine *, void *);
        virtual void Visit(TableIndexField *, void *);
        virtual void Visit(TableNameField *, void *);
        virtual void Visit(TableArrayField *, void *);
        virtual void Visit(IndexAccessor *, void *);
        virtual void Visit(MemberAccessor *, void *);
        virtual void Visit(NormalFuncCall *, void *);
        virtual void Visit(MemberFuncCall *, void *);
        virtual void Visit(FuncCallArgs *, void *);
        virtual void Visit(ExpressionList *, void *);

        SemanticAnalysisVisitor() : current_function_(nullptr) { }
        ~SemanticAnalysisVisitor()
        {
            // delete all functions
            while (current_function_)
            {
                DeleteCurrentFunction();
            }
        }

        // Enter a new function AST, and add a new LexicalFunction
        // data structure
        void EnterFunction()
        {
            auto function = new LexicalFunction;
            function->parent_ = current_function_;
            current_function_ = function;
        }

        // Leave a function AST, and delete current LexicalFunction
        // data structure
        void LeaveFunction()
        {
            DeleteCurrentFunction();
        }

        // Add a new LexicalBlock for entering a new block scope
        void EnterBlock()
        {
            assert(current_function_);
            auto block = new LexicalBlock;
            block->parent_ = current_function_->current_block_;
            current_function_->current_block_ = block;
        }

        // Delete current LexicalBlock when leaving a block scope
        void LeaveBlock()
        {
            DeleteCurrentBlock();
        }

        // Insert a name into current block
        void InsertName(const String *str)
        {
            assert(current_function_ && current_function_->current_block_);
            current_function_->current_block_->names_.insert(str->GetStdString());
        }

        // Search LexicalScoping of a name
        LexicalScoping SearchName(const String *str) const
        {
            assert(current_function_ && current_function_->current_block_);

            auto function = current_function_;
            while (function)
            {
                auto block = function->current_block_;
                while (block)
                {
                    auto it = block->names_.find(str->GetStdString());
                    if (it != block->names_.end())
                    {
                        return function == current_function_ ?
                            LexicalScoping_Local : LexicalScoping_Upvalue;
                    }

                    block = block->parent_;
                }

                function = function->parent_;
            }

            return LexicalScoping_Global;
        }

        // Set current loop AST
        void SetLoopAST(const SyntaxTree *loop)
        {
            current_function_->current_loop_ = loop;
        }

        // Get current loop AST
        const SyntaxTree *GetLoopAST() const
        {
            return current_function_->current_loop_;
        }

    private:
        void DeleteCurrentFunction()
        {
            assert(current_function_);
            // delete all blocks in current function
            DeleteBlocks();

            // delete current function
            auto function = current_function_;
            current_function_ = function->parent_;
            delete function;
        }

        void DeleteCurrentBlock()
        {
            assert(current_function_ && current_function_->current_block_);
            auto block = current_function_->current_block_;
            current_function_->current_block_ = block->parent_;
            delete block;
        }

        void DeleteBlocks()
        {
            while (current_function_->current_block_)
            {
                DeleteCurrentBlock();
            }
        }

        // Current lexical function for all names finding
        LexicalFunction *current_function_;
    };

    // Guard class, using for RAII operations.
    // e.g.
    //      {
    //          Guard g(constructor, destructor);
    //          ...
    //      }
    class Guard
    {
    public:
        Guard(const std::function<void ()> &enter,
              const std::function<void ()> &leave)
            : leave_(leave)
        {
            enter();
        }

        ~Guard()
        {
            leave_();
        }

        Guard(const Guard &) = delete;
        void operator = (const Guard &) = delete;

    private:
        std::function<void ()> leave_;
    };

#define SEMANTIC_ANALYSIS_GUARD(enter, leave)                           \
    Guard g([this]() { this->enter(); }, [this]() { this->leave(); })

#define SEMANTIC_ANALYSIS_LOOP_GUARD(loop_ast)                          \
    auto *old_loop = GetLoopAST();                                      \
    Guard l([=]() { this->SetLoopAST(loop_ast); },                      \
            [=]() { this->SetLoopAST(old_loop); })

    // For VarList AST
    struct VarListData
    {
        std::size_t var_count_;

        VarListData() : var_count_(0) { }
    };

    // For ExpList AST
    struct ExpListData
    {
        std::size_t exp_count_;

        ExpListData() : exp_count_(0) { }
    };

    // Expression type for semantic
    enum ExpType
    {
        ExpType_Unknown,    // unknown type at semantic analysis phase
        ExpType_Nil,
        ExpType_Bool,
        ExpType_Number,
        ExpType_String,
        ExpType_VarArg,
        ExpType_Table,
    };

    // Expression or variable data for semantic analysis
    struct ExpVarData
    {
        SemanticOp semantic_op_;
        ExpType exp_type_;

        explicit ExpVarData(SemanticOp semantic_op = SemanticOp_None)
            : semantic_op_(semantic_op), exp_type_(ExpType_Unknown) { }
    };

    void SemanticAnalysisVisitor::Visit(Chunk *chunk, void *data)
    {
        SEMANTIC_ANALYSIS_GUARD(EnterFunction, LeaveFunction);
        {
            SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
            chunk->block_->Accept(this, nullptr);
        }
    }

    void SemanticAnalysisVisitor::Visit(Block *block, void *data)
    {
        for (auto &stmt : block->statements_)
            stmt->Accept(this, nullptr);
        if (block->return_stmt_)
            block->return_stmt_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(ReturnStatement *ret_stmt, void *data)
    {
        if (ret_stmt->exp_list_)
        {
            ExpListData exp_list_data;
            ret_stmt->exp_list_->Accept(this, &exp_list_data);
        }
    }

    void SemanticAnalysisVisitor::Visit(BreakStatement *break_stmt, void *data)
    {
        break_stmt->loop_ = GetLoopAST();
        if (!break_stmt->loop_)
            throw SemanticException("not in any loop", break_stmt->break_);
    }

    void SemanticAnalysisVisitor::Visit(DoStatement *do_stmt, void *data)
    {
        SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
        do_stmt->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(WhileStatement *while_stmt, void *data)
    {
        SEMANTIC_ANALYSIS_LOOP_GUARD(while_stmt);
        ExpVarData exp_var_data{ SemanticOp_Read };
        while_stmt->exp_->Accept(this, &exp_var_data);

        SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
        while_stmt->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(RepeatStatement *repeat_stmt, void *data)
    {
        SEMANTIC_ANALYSIS_LOOP_GUARD(repeat_stmt);
        SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);

        ExpVarData exp_var_data{ SemanticOp_Read };
        repeat_stmt->block_->Accept(this, nullptr);
        repeat_stmt->exp_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(IfStatement *if_stmt, void *data)
    {
        ExpVarData exp_var_data{ SemanticOp_Read };
        if_stmt->exp_->Accept(this, &exp_var_data);

        {
            SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
            if_stmt->true_branch_->Accept(this, nullptr);
        }

        if (if_stmt->false_branch_)
            if_stmt->false_branch_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(ElseIfStatement *elseif_stmt, void *data)
    {
        ExpVarData exp_var_data{ SemanticOp_Read };
        elseif_stmt->exp_->Accept(this, &exp_var_data);

        {
            SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
            elseif_stmt->true_branch_->Accept(this, nullptr);
        }

        if (elseif_stmt->false_branch_)
            elseif_stmt->false_branch_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(ElseStatement *else_stmt, void *data)
    {
        SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
        else_stmt->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(NumericForStatement *num_for, void *data)
    {
        SEMANTIC_ANALYSIS_LOOP_GUARD(num_for);
        ExpVarData exp_var_data{ SemanticOp_Read };
        num_for->exp1_->Accept(this, &exp_var_data);
        num_for->exp2_->Accept(this, &exp_var_data);
        if (num_for->exp3_)
            num_for->exp3_->Accept(this, &exp_var_data);

        SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
        InsertName(num_for->name_.str_);
        num_for->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(GenericForStatement *gen_for, void *data)
    {
        SEMANTIC_ANALYSIS_LOOP_GUARD(gen_for);
        ExpListData exp_list_data;
        gen_for->exp_list_->Accept(this, &exp_list_data);

        SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);
        gen_for->name_list_->Accept(this, nullptr);
        gen_for->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(FunctionStatement *func_stmt, void *data)
    {
        func_stmt->func_name_->Accept(this, nullptr);
        func_stmt->func_body_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(FunctionName *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(LocalFunctionStatement *l_func_stmt, void *data)
    {
        InsertName(l_func_stmt->name_.str_);
        l_func_stmt->func_body_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(LocalNameListStatement *l_namelist_stmt, void *data)
    {
        l_namelist_stmt->name_list_->Accept(this, nullptr);
        if (l_namelist_stmt->exp_list_)
        {
            ExpListData exp_list_data;
            l_namelist_stmt->exp_list_->Accept(this, &exp_list_data);
        }
    }

    void SemanticAnalysisVisitor::Visit(AssignmentStatement *assign_stmt, void *data)
    {
        VarListData var_list_data;
        ExpListData exp_list_data;
        assign_stmt->var_list_->Accept(this, &var_list_data);
        assign_stmt->exp_list_->Accept(this, &exp_list_data);
        assign_stmt->var_count_ = var_list_data.var_count_;
        assign_stmt->exp_count_ = exp_list_data.exp_count_;
    }

    void SemanticAnalysisVisitor::Visit(VarList *var_list, void *data)
    {
        ExpVarData exp_var_data{ SemanticOp_Write };
        for (auto &var : var_list->var_list_)
            var->Accept(this, &exp_var_data);
        static_cast<VarListData *>(data)->var_count_ = var_list->var_list_.size();
    }

    void SemanticAnalysisVisitor::Visit(Terminator *term, void *data)
    {
        // Current term read and write semantic
        auto exp_var_data = static_cast<ExpVarData *>(data);
        term->semantic_ = exp_var_data->semantic_op_;
        if (term->token_.token_ != Token_Id)
            assert(exp_var_data->semantic_op_ == SemanticOp_Read);

        // Set Expression type
        switch (term->token_.token_)
        {
            case Token_Nil: exp_var_data->exp_type_ = ExpType_Nil; break;
            case Token_Id: exp_var_data->exp_type_ = ExpType_Unknown; break;
            case Token_Number: exp_var_data->exp_type_ = ExpType_Number; break;
            case Token_String: exp_var_data->exp_type_ = ExpType_String; break;
            case Token_VarArg: exp_var_data->exp_type_ = ExpType_VarArg; break;
            case Token_True: case Token_False: exp_var_data->exp_type_ = ExpType_Bool; break;
        }

        // Search lexical scoping of name
        if (term->token_.token_ == Token_Id)
            term->scoping_ = SearchName(term->token_.str_);
    }

    void SemanticAnalysisVisitor::Visit(BinaryExpression *binary_exp, void *data)
    {
        // Binary expression is read semantic
        ExpVarData l_exp_var_data{ SemanticOp_Read };
        ExpVarData r_exp_var_data{ SemanticOp_Read };
        binary_exp->left_->Accept(this, &l_exp_var_data);
        binary_exp->right_->Accept(this, &r_exp_var_data);

        auto parent_exp_var_data = static_cast<ExpVarData *>(data);
        switch (binary_exp->op_token_.token_)
        {
            case '+': case '-': case '*': case '/': case '^': case '%':
                if (l_exp_var_data.exp_type_ != ExpType_Unknown &&
                    l_exp_var_data.exp_type_ != ExpType_Number)
                    throw SemanticException("left expression of binary operator is not number",
                                            binary_exp->op_token_);
                if (r_exp_var_data.exp_type_ != ExpType_Unknown &&
                    r_exp_var_data.exp_type_ != ExpType_Number)
                    throw SemanticException("right expression of binary operator is not number",
                                            binary_exp->op_token_);
                parent_exp_var_data->exp_type_ = ExpType_Number;
                break;
            case '<': case '>': case Token_LessEqual: case Token_BigEqual:
                if (l_exp_var_data.exp_type_ != ExpType_Unknown &&
                    r_exp_var_data.exp_type_ != ExpType_Unknown)
                {
                    if (l_exp_var_data.exp_type_ != r_exp_var_data.exp_type_)
                        throw SemanticException("compare different expression type",
                                                binary_exp->op_token_);
                    else if (l_exp_var_data.exp_type_ != ExpType_Number &&
                             l_exp_var_data.exp_type_ != ExpType_String)
                        throw SemanticException("can not compare operands",
                                                binary_exp->op_token_);
                }
                parent_exp_var_data->exp_type_ = ExpType_Bool;
                break;
            case Token_Concat:
                if (l_exp_var_data.exp_type_ != ExpType_Unknown &&
                    r_exp_var_data.exp_type_ != ExpType_Unknown)
                {
                    if (!((l_exp_var_data.exp_type_ == ExpType_String &&
                           r_exp_var_data.exp_type_ == ExpType_String) ||
                          (l_exp_var_data.exp_type_ == ExpType_String &&
                           r_exp_var_data.exp_type_ == ExpType_Number) ||
                          (l_exp_var_data.exp_type_ == ExpType_Number &&
                           r_exp_var_data.exp_type_ == ExpType_String)))
                        throw SemanticException("can not concat operands",
                                                binary_exp->op_token_);
                }
                parent_exp_var_data->exp_type_ = ExpType_String;
                break;
            case Token_NotEqual: case Token_Equal:
                parent_exp_var_data->exp_type_ = ExpType_Bool;
                break;
            case Token_And: case Token_Or:
                break;
            default:
                break;
        }
    }

    void SemanticAnalysisVisitor::Visit(UnaryExpression *unary_exp, void *data)
    {
        // Unary expression is read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        unary_exp->exp_->Accept(this, &exp_var_data);

        // Expression type
        if (exp_var_data.exp_type_ != ExpType_Unknown)
        {
            switch (unary_exp->op_token_.token_)
            {
                case '-':
                    if (exp_var_data.exp_type_ != ExpType_Number)
                        throw SemanticException("operand is not number",
                                                unary_exp->op_token_);
                    break;
                case '#':
                    if (exp_var_data.exp_type_ != ExpType_Table &&
                        exp_var_data.exp_type_ != ExpType_String)
                        throw SemanticException("operand is not table or string",
                                                unary_exp->op_token_);
                    break;
                default:
                    break;
            }
        }

        auto parent_exp_var_data = static_cast<ExpVarData *>(data);
        if (unary_exp->op_token_.token_ == '-' ||
            unary_exp->op_token_.token_ == '#')
            parent_exp_var_data->exp_type_ = ExpType_Number;
        else if (unary_exp->op_token_.token_ == Token_Not)
            parent_exp_var_data->exp_type_ = ExpType_Bool;
        else
            assert(!"unexpect unary operator");
    }

    void SemanticAnalysisVisitor::Visit(FunctionBody *func_body, void *data)
    {
        SEMANTIC_ANALYSIS_GUARD(EnterFunction, LeaveFunction);
        {
            SEMANTIC_ANALYSIS_GUARD(EnterBlock, LeaveBlock);

            if (func_body->param_list_)
                func_body->param_list_->Accept(this, nullptr);

            func_body->block_->Accept(this, nullptr);
        }
    }

    void SemanticAnalysisVisitor::Visit(ParamList *par_list, void *data)
    {
        if (par_list->name_list_)
            par_list->name_list_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(NameList *name_list, void *data)
    {
        for (const auto &name : name_list->names_)
            InsertName(name.str_);
    }

    void SemanticAnalysisVisitor::Visit(TableDefine *table_def, void *data)
    {
        for (auto &field : table_def->fields_)
            field->Accept(this, nullptr);

        // Set Expression type
        static_cast<ExpVarData *>(data)->exp_type_ = ExpType_Table;
    }

    void SemanticAnalysisVisitor::Visit(TableIndexField *table_i_field, void *data)
    {
        // Table index_ and value_ expressions are read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        table_i_field->index_->Accept(this, &exp_var_data);
        table_i_field->value_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(TableNameField *table_n_field, void *data)
    {
        // Table value_ expression is read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        table_n_field->value_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(TableArrayField *table_a_field, void *data)
    {
        // Table value_ expression is read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        table_a_field->value_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(IndexAccessor *i_accessor, void *data)
    {
        // Set this IndexAccessor semantic from parent's semantic data
        i_accessor->semantic_ = static_cast<ExpVarData *>(data)->semantic_op_;

        // table_ and index_ expressions are read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        i_accessor->table_->Accept(this, &exp_var_data);
        i_accessor->index_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(MemberAccessor *m_accessor, void *data)
    {
        // Set this MemberAccessor semantic from parent's semantic data
        m_accessor->semantic_ = static_cast<ExpVarData *>(data)->semantic_op_;

        // table_ expression is read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        m_accessor->table_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(NormalFuncCall *n_func_call, void *data)
    {
        // Function call must be read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        n_func_call->caller_->Accept(this, &exp_var_data);
        n_func_call->args_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(MemberFuncCall *m_func_call, void *data)
    {
        // Function call must be read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        m_func_call->caller_->Accept(this, &exp_var_data);
        m_func_call->args_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(FuncCallArgs *call_args, void *data)
    {
        if (call_args->type_ == FuncCallArgs::ExpList)
        {
            if (call_args->arg_)
            {
                ExpListData exp_list_data;
                call_args->arg_->Accept(this, &exp_list_data);
            }
        }
        else
        {
            call_args->arg_->Accept(this, nullptr);
        }
    }

    void SemanticAnalysisVisitor::Visit(ExpressionList *exp_list, void *data)
    {
        // Expressions in ExpressionList must be read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        for (auto &exp : exp_list->exp_list_)
            exp->Accept(this, &exp_var_data);

        static_cast<ExpListData *>(data)->exp_count_ = exp_list->exp_list_.size();
    }

    void SemanticAnalysis(SyntaxTree *root)
    {
        assert(root);
        SemanticAnalysisVisitor semantic_analysis;
        root->Accept(&semantic_analysis, nullptr);
    }
} // namespace luna
