#include "Visitor.h"
#include "State.h"
#include "Function.h"

namespace luna
{
    class CodeGenerateVisitor : public Visitor
    {
    public:
        explicit CodeGenerateVisitor(State *state);

        virtual void Visit(Chunk *);
        virtual void Visit(Block *);
        virtual void Visit(ReturnStatement *);
        virtual void Visit(BreakStatement *);
        virtual void Visit(DoStatement *);
        virtual void Visit(WhileStatement *);
        virtual void Visit(RepeatStatement *);
        virtual void Visit(IfStatement *);
        virtual void Visit(ElseIfStatement *);
        virtual void Visit(ElseStatement *);
        virtual void Visit(NumericForStatement *);
        virtual void Visit(GenericForStatement *);
        virtual void Visit(FunctionStatement *);
        virtual void Visit(FunctionName *);
        virtual void Visit(LocalFunctionStatement *);
        virtual void Visit(LocalNameListStatement *);
        virtual void Visit(AssignmentStatement *);
        virtual void Visit(VarList *);
        virtual void Visit(Terminator *);
        virtual void Visit(BinaryExpression *);
        virtual void Visit(UnaryExpression *);
        virtual void Visit(FunctionBody *);
        virtual void Visit(ParamList *);
        virtual void Visit(NameList *);
        virtual void Visit(TableDefine *);
        virtual void Visit(TableIndexField *);
        virtual void Visit(TableNameField *);
        virtual void Visit(TableArrayField *);
        virtual void Visit(IndexAccessor *);
        virtual void Visit(MemberAccessor *);
        virtual void Visit(NormalFuncCall *);
        virtual void Visit(MemberFuncCall *);
        virtual void Visit(ExpressionList *);

    private:
        State *state_;

        // current function
        Function *func_;
    };

    CodeGenerateVisitor::CodeGenerateVisitor(State *state)
        : state_(state),
          func_(nullptr)
    {
    }

    void CodeGenerateVisitor::Visit(Chunk *chunk)
    {
        func_ = state_->NewFunction();
        func_->SetBaseInfo(chunk->module_, 0);
    }

    std::unique_ptr<Visitor> GenerateVisitor(State *state)
    {
        return std::unique_ptr<Visitor>(new CodeGenerateVisitor(state));
    }
} // namespace luna
