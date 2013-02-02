#ifndef VISITOR_H
#define VISITOR_H

#include "SyntaxTree.h"

namespace luna
{
    class Visitor
    {
    public:
        virtual void Visit(Chunk *) = 0;
        virtual void Visit(Block *) = 0;
        virtual void Visit(ReturnStatement *) = 0;
        virtual void Visit(BreakStatement *) = 0;
        virtual void Visit(DoStatement *) = 0;
        virtual void Visit(WhileStatement *) = 0;
        virtual void Visit(RepeatStatement *) = 0;
        virtual void Visit(IfStatement *) = 0;
        virtual void Visit(ElseIfStatement *) = 0;
        virtual void Visit(ElseStatement *) = 0;
        virtual void Visit(NumericForStatement *) = 0;
        virtual void Visit(GenericForStatement *) = 0;
        virtual void Visit(FunctionStatement *) = 0;
        virtual void Visit(FunctionName *) = 0;
        virtual void Visit(LocalFunctionStatement *) = 0;
        virtual void Visit(LocalNameListStatement *) = 0;
        virtual void Visit(AssignmentStatement *) = 0;
        virtual void Visit(VarList *) = 0;
        virtual void Visit(Terminator *) = 0;
        virtual void Visit(BinaryExpression *) = 0;
        virtual void Visit(UnaryExpression *) = 0;
        virtual void Visit(FunctionBody *) = 0;
        virtual void Visit(ParamList *) = 0;
        virtual void Visit(NameList *) = 0;
        virtual void Visit(TableDefine *) = 0;
        virtual void Visit(TableIndexField *) = 0;
        virtual void Visit(TableNameField *) = 0;
        virtual void Visit(TableArrayField *) = 0;
        virtual void Visit(IndexAccessor *) = 0;
        virtual void Visit(MemberAccessor *) = 0;
        virtual void Visit(NormalFuncCall *) = 0;
        virtual void Visit(MemberFuncCall *) = 0;
        virtual void Visit(ExpressionList *) = 0;
    };
} // namespace luna

#endif // VISITOR_H
