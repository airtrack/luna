#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "Token.h"
#include <memory>
#include <vector>

namespace luna
{
#define SYNTAX_TREE_ACCEPT_VISITOR()                \
    virtual void Accept(Visitor *v, void *data) = 0
#define SYNTAX_TREE_ACCEPT_VISITOR_DECL()           \
    virtual void Accept(Visitor *v, void *data)

    // Expression or variable operation semantic
    enum SemanticOp
    {
        SemanticOp_None,
        SemanticOp_Read,
        SemanticOp_Write,
    };

    // Expression or variable lexical scoping
    enum LexicalScoping
    {
        LexicalScoping_Unknown,
        LexicalScoping_Global,          // Expression or variable in global table
        LexicalScoping_Upvalue,         // Expression or variable is upvalue
        LexicalScoping_Local,           // Expression or variable in current function
    };

    class String;
    class Visitor;

    // AST base class, all AST node derived from this class and
    // provide Visitor to Accept itself.
    class SyntaxTree
    {
    public:
        virtual ~SyntaxTree() { }
        SYNTAX_TREE_ACCEPT_VISITOR();
    };

    class Chunk : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> block_;
        String *module_;

        Chunk(std::unique_ptr<SyntaxTree> block,
              String *module)
            : block_(std::move(block)),
              module_(module)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class Block : public SyntaxTree
    {
    public:
        std::vector<std::unique_ptr<SyntaxTree>> statements_;
        std::unique_ptr<SyntaxTree> return_stmt_;

        Block() { }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class ReturnStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> exp_list_;

        ReturnStatement() { }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class BreakStatement : public SyntaxTree
    {
    public:
        TokenDetail break_;

        explicit BreakStatement(const TokenDetail &b)
            : break_(b)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class DoStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> block_;

        explicit DoStatement(std::unique_ptr<SyntaxTree> block)
            : block_(std::move(block))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class WhileStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> exp_;
        std::unique_ptr<SyntaxTree> block_;

        WhileStatement(std::unique_ptr<SyntaxTree> exp,
                       std::unique_ptr<SyntaxTree> block)
            : exp_(std::move(exp)), block_(std::move(block))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class RepeatStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> block_;
        std::unique_ptr<SyntaxTree> exp_;

        RepeatStatement(std::unique_ptr<SyntaxTree> block,
                        std::unique_ptr<SyntaxTree> exp)
            : block_(std::move(block)), exp_(std::move(exp))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class IfStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> exp_;
        std::unique_ptr<SyntaxTree> true_branch_;
        std::unique_ptr<SyntaxTree> false_branch_;

        IfStatement(std::unique_ptr<SyntaxTree> exp,
                    std::unique_ptr<SyntaxTree> true_branch,
                    std::unique_ptr<SyntaxTree> false_branch)
            : exp_(std::move(exp)),
              true_branch_(std::move(true_branch)),
              false_branch_(std::move(false_branch))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class ElseIfStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> exp_;
        std::unique_ptr<SyntaxTree> true_branch_;
        std::unique_ptr<SyntaxTree> false_branch_;

        ElseIfStatement(std::unique_ptr<SyntaxTree> exp,
                        std::unique_ptr<SyntaxTree> true_branch,
                        std::unique_ptr<SyntaxTree> false_branch)
            : exp_(std::move(exp)),
              true_branch_(std::move(true_branch)),
              false_branch_(std::move(false_branch))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class ElseStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> block_;

        ElseStatement(std::unique_ptr<SyntaxTree> block)
            : block_(std::move(block))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class NumericForStatement : public SyntaxTree
    {
    public:
        TokenDetail name_;
        std::unique_ptr<SyntaxTree> exp1_;
        std::unique_ptr<SyntaxTree> exp2_;
        std::unique_ptr<SyntaxTree> exp3_;
        std::unique_ptr<SyntaxTree> block_;

        NumericForStatement(const TokenDetail &name,
                            std::unique_ptr<SyntaxTree> exp1,
                            std::unique_ptr<SyntaxTree> exp2,
                            std::unique_ptr<SyntaxTree> exp3,
                            std::unique_ptr<SyntaxTree> block)
            : name_(name),
              exp1_(std::move(exp1)),
              exp2_(std::move(exp2)),
              exp3_(std::move(exp3)),
              block_(std::move(block))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class GenericForStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> name_list_;
        std::unique_ptr<SyntaxTree> exp_list_;
        std::unique_ptr<SyntaxTree> block_;

        GenericForStatement(std::unique_ptr<SyntaxTree> name_list,
                            std::unique_ptr<SyntaxTree> exp_list,
                            std::unique_ptr<SyntaxTree> block)
            : name_list_(std::move(name_list)),
              exp_list_(std::move(exp_list)),
              block_(std::move(block))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class FunctionStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> func_name_;
        std::unique_ptr<SyntaxTree> func_body_;

        FunctionStatement(std::unique_ptr<SyntaxTree> func_name,
                          std::unique_ptr<SyntaxTree> func_body)
            : func_name_(std::move(func_name)), func_body_(std::move(func_body))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class FunctionName : public SyntaxTree
    {
    public:
        std::vector<TokenDetail> names_;
        TokenDetail member_name_;

        FunctionName() { }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class LocalFunctionStatement : public SyntaxTree
    {
    public:
        TokenDetail name_;
        std::unique_ptr<SyntaxTree> func_body_;

        LocalFunctionStatement(const TokenDetail &name,
                               std::unique_ptr<SyntaxTree> func_body)
            : name_(name), func_body_(std::move(func_body))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class LocalNameListStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> name_list_;
        std::unique_ptr<SyntaxTree> exp_list_;

        LocalNameListStatement(std::unique_ptr<SyntaxTree> name_list,
                               std::unique_ptr<SyntaxTree> exp_list)
            : name_list_(std::move(name_list)), exp_list_(std::move(exp_list))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class AssignmentStatement : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> var_list_;
        std::unique_ptr<SyntaxTree> exp_list_;

        // For semantic
        std::size_t var_count_;
        std::size_t exp_count_;

        AssignmentStatement(std::unique_ptr<SyntaxTree> var_list,
                            std::unique_ptr<SyntaxTree> exp_list)
            : var_list_(std::move(var_list)), exp_list_(std::move(exp_list)),
              var_count_(0), exp_count_(0)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class VarList : public SyntaxTree
    {
    public:
        std::vector<std::unique_ptr<SyntaxTree>> var_list_;

        VarList() { }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class Terminator : public SyntaxTree
    {
    public:
        TokenDetail token_;

        // For semantic
        SemanticOp semantic_;
        LexicalScoping scoping_;

        Terminator() { }
        explicit Terminator(const TokenDetail &token)
            : token_(token), semantic_(SemanticOp_None),
              scoping_(LexicalScoping_Unknown) { }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class BinaryExpression : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> left_;
        std::unique_ptr<SyntaxTree> right_;
        TokenDetail op_token_;

        BinaryExpression() { }
        BinaryExpression(std::unique_ptr<SyntaxTree> left,
                         std::unique_ptr<SyntaxTree> right,
                         const TokenDetail &op)
            : left_(std::move(left)), right_(std::move(right)), op_token_(op)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class UnaryExpression : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> exp_;
        TokenDetail op_token_;

        UnaryExpression() { }
        UnaryExpression(std::unique_ptr<SyntaxTree> exp,
                        const TokenDetail &op)
            : exp_(std::move(exp)), op_token_(op)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class FunctionBody : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> param_list_;
        std::unique_ptr<SyntaxTree> block_;

        FunctionBody() { }
        FunctionBody(std::unique_ptr<SyntaxTree> param_list,
                     std::unique_ptr<SyntaxTree> block)
            : param_list_(std::move(param_list)), block_(std::move(block))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class ParamList : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> name_list_;
        bool vararg_;

        ParamList(std::unique_ptr<SyntaxTree> name_list, bool vararg)
            : name_list_(std::move(name_list)), vararg_(vararg)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class NameList : public SyntaxTree
    {
    public:
        std::vector<TokenDetail> names_;

        NameList() { }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class TableDefine : public SyntaxTree
    {
    public:
        std::vector<std::unique_ptr<SyntaxTree>> fields_;

        TableDefine() { }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class TableIndexField : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> index_;
        std::unique_ptr<SyntaxTree> value_;

        TableIndexField(std::unique_ptr<SyntaxTree> index,
                        std::unique_ptr<SyntaxTree> value)
            : index_(std::move(index)), value_(std::move(value))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class TableNameField : public SyntaxTree
    {
    public:
        TokenDetail name_;
        std::unique_ptr<SyntaxTree> value_;

        TableNameField(const TokenDetail &name,
                       std::unique_ptr<SyntaxTree> value)
            : name_(name), value_(std::move(value))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class TableArrayField : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> value_;

        explicit TableArrayField(std::unique_ptr<SyntaxTree> value)
            : value_(std::move(value))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class IndexAccessor : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> table_;
        std::unique_ptr<SyntaxTree> index_;

        // For semantic
        SemanticOp semantic_;

        IndexAccessor(std::unique_ptr<SyntaxTree> table,
                      std::unique_ptr<SyntaxTree> index)
            : table_(std::move(table)), index_(std::move(index)),
              semantic_(SemanticOp_None)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class MemberAccessor : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> table_;
        TokenDetail member_;

        // For semantic
        SemanticOp semantic_;

        MemberAccessor(std::unique_ptr<SyntaxTree> table,
                       const TokenDetail &member)
            : table_(std::move(table)), member_(member),
              semantic_(SemanticOp_None)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class NormalFuncCall : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> caller_;
        std::unique_ptr<SyntaxTree> args_;

        NormalFuncCall(std::unique_ptr<SyntaxTree> caller,
                       std::unique_ptr<SyntaxTree> args)
            : caller_(std::move(caller)), args_(std::move(args))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class MemberFuncCall : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> caller_;
        TokenDetail member_;
        std::unique_ptr<SyntaxTree> args_;

        MemberFuncCall(std::unique_ptr<SyntaxTree> caller,
                       const TokenDetail &member,
                       std::unique_ptr<SyntaxTree> args)
            : caller_(std::move(caller)), member_(member), args_(std::move(args))
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class FuncCallArgs : public SyntaxTree
    {
    public:
        std::unique_ptr<SyntaxTree> arg_;
        enum ArgType { ExpList, Table, String } type_;

        FuncCallArgs(std::unique_ptr<SyntaxTree> arg,
                     ArgType type)
            : arg_(std::move(arg)), type_(type)
        {
        }

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };

    class ExpressionList : public SyntaxTree
    {
    public:
        std::vector<std::unique_ptr<SyntaxTree>> exp_list_;

        SYNTAX_TREE_ACCEPT_VISITOR_DECL();
    };
} // namespace luna

#endif // SYNTAX_TREE_H
