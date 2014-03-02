#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "Lex.h"
#include "Parser.h"
#include "State.h"
#include "String.h"
#include "TextInStream.h"
#include "Exception.h"
#include "Visitor.h"
#include <functional>
#include <type_traits>

class ParserWrapper
{
public:
    explicit ParserWrapper(const std::string &str = "")
        : iss_(str), state_(), name_("parser"),
          lexer_(&state_, &name_, std::bind(&io::text::InStringStream::GetChar, &iss_))
    {
    }

    void SetInput(const std::string &input)
    {
        iss_.SetInputString(input);
    }

    bool IsEOF()
    {
        luna::TokenDetail detail;
        return lexer_.GetToken(&detail) == luna::Token_EOF;
    }

    std::unique_ptr<luna::SyntaxTree> Parse()
    {
        return parser_.Parse(&lexer_);
    }

    luna::State * GetState()
    {
        return &state_;
    }

private:
    io::text::InStringStream iss_;
    luna::State state_;
    luna::String name_;
    luna::Lexer lexer_;
    luna::Parser parser_;
};

#define MATCH_AST_TYPE(ast, not_match_stmt)                             \
    if (!the_ast_node_)                                                 \
    {                                                                   \
        auto f = [&]() not_match_stmt;                                  \
        SetResult(typename std::conditional<                            \
            std::is_same<std::remove_reference<decltype(*ast)>::type,   \
                         ASTType>::value,                               \
            std::true_type, std::false_type>::type(), ast, f);          \
    }

template<typename ASTType, typename FinderType>
class ASTFinder : public luna::Visitor
{
public:
    explicit ASTFinder(const FinderType &finder)
        : the_ast_node_(nullptr), finder_(finder) { }

    ASTType * GetResult() const
    {
        return the_ast_node_;
    }

    virtual void Visit(luna::Chunk *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->block_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::Block *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            for (auto &stmt : ast->statements_)
                stmt->Accept(this, nullptr);
            if (ast->return_stmt_)
                ast->return_stmt_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::ReturnStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            if (ast->exp_list_)
                ast->exp_list_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::BreakStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {})
    }

    virtual void Visit(luna::DoStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->block_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::WhileStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->exp_->Accept(this, nullptr);
            ast->block_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::RepeatStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->block_->Accept(this, nullptr);
            ast->exp_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::IfStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->exp_->Accept(this, nullptr);
            ast->true_branch_->Accept(this, nullptr);
            if (ast->false_branch_)
                ast->false_branch_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::ElseIfStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->exp_->Accept(this, nullptr);
            ast->true_branch_->Accept(this, nullptr);
            if (ast->false_branch_)
                ast->false_branch_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::ElseStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->block_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::NumericForStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->exp1_->Accept(this, nullptr);
            ast->exp2_->Accept(this, nullptr);
            if (ast->exp3_)
                ast->exp3_->Accept(this, nullptr);
            ast->block_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::GenericForStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->name_list_->Accept(this, nullptr);
            ast->exp_list_->Accept(this, nullptr);
            ast->block_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::FunctionStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->func_name_->Accept(this, nullptr);
            ast->func_body_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::FunctionName *ast, void *)
    {
        MATCH_AST_TYPE(ast, {})
    }

    virtual void Visit(luna::LocalFunctionStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->func_body_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::LocalNameListStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->name_list_->Accept(this, nullptr);
            if (ast->exp_list_)
                ast->exp_list_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::AssignmentStatement *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->var_list_->Accept(this, nullptr);
            ast->exp_list_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::VarList *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            for (auto &var : ast->var_list_)
                var->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::Terminator *ast, void *)
    {
        MATCH_AST_TYPE(ast, {})
    }

    virtual void Visit(luna::BinaryExpression *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->left_->Accept(this, nullptr);
            ast->right_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::UnaryExpression *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->exp_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::FunctionBody *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            if (ast->param_list_)
                ast->param_list_->Accept(this, nullptr);
            ast->block_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::ParamList *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            if (ast->name_list_)
                ast->name_list_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::NameList *ast, void *)
    {
        MATCH_AST_TYPE(ast, {})
    }

    virtual void Visit(luna::TableDefine *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            for (auto &field : ast->fields_)
                field->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::TableIndexField *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->index_->Accept(this, nullptr);
            ast->value_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::TableNameField *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->value_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::TableArrayField *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->value_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::IndexAccessor *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->table_->Accept(this, nullptr);
            ast->index_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::MemberAccessor *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->table_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::NormalFuncCall *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->caller_->Accept(this, nullptr);
            ast->args_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::MemberFuncCall *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            ast->caller_->Accept(this, nullptr);
            ast->args_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::FuncCallArgs *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            if (ast->arg_)
                ast->arg_->Accept(this, nullptr);
        })
    }

    virtual void Visit(luna::ExpressionList *ast, void *)
    {
        MATCH_AST_TYPE(ast, {
            for (auto &exp : ast->exp_list_)
                exp->Accept(this, nullptr);
        })
    }

private:
    template<typename Type, typename Func>
    void SetResult(std::true_type, Type *t, const Func &op)
    {
        if (finder_(t))
            the_ast_node_ = t;
        else
            op();
    }

    template<typename Type, typename Func>
    void SetResult(std::false_type, Type *t, const Func &op)
    {
        op();
    }

    ASTType *the_ast_node_;
    FinderType finder_;
};

template<typename ASTType, typename FinderType>
ASTType * ASTFind(const std::unique_ptr<luna::SyntaxTree> &root,
                  const FinderType &finder)
{
    ASTFinder<ASTType, FinderType> ast_finder(finder);
    root->Accept(&ast_finder, nullptr);
    return ast_finder.GetResult();
}

struct FindName
{
    FindName(const std::string &name) : name_(name) { }

    bool operator () (const luna::Terminator *term) const
    {
        if (term->token_.token_ == luna::Token_Id)
            return term->token_.str_->GetStdString() == name_;
        else
            return false;
    }

    std::string name_;
};

struct AcceptAST
{
    bool operator () (const luna::SyntaxTree *) const
    { return true; }
};

#endif // TEST_COMMON_H
