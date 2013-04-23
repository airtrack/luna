#ifndef VISITOR_H
#define VISITOR_H

#include "SyntaxTree.h"
#include <memory>

namespace luna
{
    class State;

    class Visitor
    {
    public:
        virtual ~Visitor() { }
        virtual void Visit(Chunk *, void *) = 0;
        virtual void Visit(Block *, void *) = 0;
        virtual void Visit(ReturnStatement *, void *) = 0;
        virtual void Visit(BreakStatement *, void *) = 0;
        virtual void Visit(DoStatement *, void *) = 0;
        virtual void Visit(WhileStatement *, void *) = 0;
        virtual void Visit(RepeatStatement *, void *) = 0;
        virtual void Visit(IfStatement *, void *) = 0;
        virtual void Visit(ElseIfStatement *, void *) = 0;
        virtual void Visit(ElseStatement *, void *) = 0;
        virtual void Visit(NumericForStatement *, void *) = 0;
        virtual void Visit(GenericForStatement *, void *) = 0;
        virtual void Visit(FunctionStatement *, void *) = 0;
        virtual void Visit(FunctionName *, void *) = 0;
        virtual void Visit(LocalFunctionStatement *, void *) = 0;
        virtual void Visit(LocalNameListStatement *, void *) = 0;
        virtual void Visit(AssignmentStatement *, void *) = 0;
        virtual void Visit(VarList *, void *) = 0;
        virtual void Visit(Terminator *, void *) = 0;
        virtual void Visit(BinaryExpression *, void *) = 0;
        virtual void Visit(UnaryExpression *, void *) = 0;
        virtual void Visit(FunctionBody *, void *) = 0;
        virtual void Visit(ParamList *, void *) = 0;
        virtual void Visit(NameList *, void *) = 0;
        virtual void Visit(TableDefine *, void *) = 0;
        virtual void Visit(TableIndexField *, void *) = 0;
        virtual void Visit(TableNameField *, void *) = 0;
        virtual void Visit(TableArrayField *, void *) = 0;
        virtual void Visit(IndexAccessor *, void *) = 0;
        virtual void Visit(MemberAccessor *, void *) = 0;
        virtual void Visit(NormalFuncCall *, void *) = 0;
        virtual void Visit(MemberFuncCall *, void *) = 0;
        virtual void Visit(FuncCallArgs *, void *) = 0;
        virtual void Visit(ExpressionList *, void *) = 0;
    };

    std::unique_ptr<Visitor> GenerateVisitor(State *state);
} // namespace luna

#endif // VISITOR_H
