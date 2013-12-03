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

    void SemanticAnalysisVisitor::Visit(Chunk *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(Block *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(ReturnStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(BreakStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(DoStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(WhileStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(RepeatStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(IfStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(ElseIfStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(ElseStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(NumericForStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(GenericForStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(FunctionStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(FunctionName *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(LocalFunctionStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(LocalNameListStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(AssignmentStatement *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(VarList *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(Terminator *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(BinaryExpression *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(UnaryExpression *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(FunctionBody *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(ParamList *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(NameList *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(TableDefine *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(TableIndexField *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(TableNameField *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(TableArrayField *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(IndexAccessor *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(MemberAccessor *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(NormalFuncCall *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(MemberFuncCall *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(FuncCallArgs *, void *data)
    {
    }

    void SemanticAnalysisVisitor::Visit(ExpressionList *, void *data)
    {
    }

    void SemanticAnalysis(SyntaxTree *root)
    {
        assert(root);
        SemanticAnalysisVisitor semantic_analysis;
        root->Accept(&semantic_analysis, nullptr);
    }
} // namespace luna
