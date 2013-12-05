#include "SemanticAnalysis.h"
#include "Visitor.h"
#include <assert.h>

namespace luna
{
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
    };

    struct VarListData
    {
        std::size_t var_count_;
    };

    struct ExpVarData
    {
        SemanticOp semantic_op_;
    };

    void SemanticAnalysisVisitor::Visit(Chunk *chunk, void *data)
    {
        chunk->block_->Accept(this, nullptr);
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
            ret_stmt->exp_list_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(BreakStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(DoStatement *do_stmt, void *data)
    {
        do_stmt->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(WhileStatement *while_stmt, void *data)
    {
        while_stmt->exp_->Accept(this, nullptr);
        while_stmt->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(RepeatStatement *repeat_stmt, void *data)
    {
        repeat_stmt->block_->Accept(this, nullptr);
        repeat_stmt->exp_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(IfStatement *if_stmt, void *data)
    {
        if_stmt->exp_->Accept(this, nullptr);
        if_stmt->true_branch_->Accept(this, nullptr);
        if (if_stmt->false_branch_)
            if_stmt->false_branch_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(ElseIfStatement *elseif_stmt, void *data)
    {
        elseif_stmt->exp_->Accept(this, nullptr);
        elseif_stmt->true_branch_->Accept(this, nullptr);
        if (elseif_stmt->false_branch_)
            elseif_stmt->false_branch_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(ElseStatement *else_stmt, void *data)
    {
        else_stmt->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(NumericForStatement *num_for, void *data)
    {
        num_for->exp1_->Accept(this, nullptr);
        num_for->exp2_->Accept(this, nullptr);
        if (num_for->exp3_)
            num_for->exp3_->Accept(this, nullptr);
        num_for->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(GenericForStatement *gen_for, void *data)
    {
        gen_for->name_list_->Accept(this, nullptr);
        gen_for->exp_list_->Accept(this, nullptr);
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
        l_func_stmt->func_body_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(LocalNameListStatement *l_namelist_stmt, void *data)
    {
        l_namelist_stmt->name_list_->Accept(this, nullptr);
        if (l_namelist_stmt->exp_list_)
            l_namelist_stmt->exp_list_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(AssignmentStatement *assign_stmt, void *data)
    {
        VarListData var_list_data{ 0 };
        assign_stmt->var_list_->Accept(this, &var_list_data);
        assign_stmt->exp_list_->Accept(this, nullptr);
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
        auto exp_var_data = static_cast<ExpVarData *>(data);
        term->semantic_ = exp_var_data->semantic_op_;
        if (term->token_.token_ != Token_Id)
            assert(exp_var_data->semantic_op_ == SemanticOp_Read);
    }

    void SemanticAnalysisVisitor::Visit(BinaryExpression *binary_exp, void *data)
    {
        // Binary expression is read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        binary_exp->left_->Accept(this, &exp_var_data);
        binary_exp->right_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(UnaryExpression *unary_exp, void *data)
    {
        // Unary expression is read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        unary_exp->exp_->Accept(this, &exp_var_data);
    }

    void SemanticAnalysisVisitor::Visit(FunctionBody *func_body, void *data)
    {
        if (func_body->param_list_)
            func_body->param_list_->Accept(this, nullptr);
        func_body->block_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(ParamList *par_list, void *data)
    {
        if (par_list->name_list_)
            par_list->name_list_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(NameList *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(TableDefine *table_def, void *data)
    {
        for (auto &field : table_def->fields_)
            field->Accept(this, nullptr);
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
        call_args->arg_->Accept(this, nullptr);
    }

    void SemanticAnalysisVisitor::Visit(ExpressionList *exp_list, void *data)
    {
        // Expressions in ExpressionList must be read semantic
        ExpVarData exp_var_data{ SemanticOp_Read };
        for (auto &exp : exp_list->exp_list_)
            exp->Accept(this, &exp_var_data);
    }

    void SemanticAnalysis(SyntaxTree *root)
    {
        assert(root);
        SemanticAnalysisVisitor semantic_analysis;
        root->Accept(&semantic_analysis, nullptr);
    }
} // namespace luna
