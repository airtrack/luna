#include "SyntaxTree.h"
#include "Visitor.h"

namespace luna
{
    void Chunk::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void Block::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void ReturnStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void BreakStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void DoStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void WhileStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void RepeatStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void IfStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void ElseIfStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void ElseStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void NumericForStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void GenericForStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void FunctionStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void FunctionName::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void LocalFunctionStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void LocalNameListStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void AssignmentStatement::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void VarList::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void Terminator::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void BinaryExpression::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void UnaryExpression::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void FunctionBody::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void ParamList::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void NameList::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void TableDefine::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void TableIndexField::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void TableNameField::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void TableArrayField::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void IndexAccessor::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void MemberAccessor::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void NormalFuncCall::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void MemberFuncCall::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void FuncCallArgs::Accept(Visitor *v)
    {
        v->Visit(this);
    }

    void ExpressionList::Accept(Visitor *v)
    {
        v->Visit(this);
    }
} // namespace luna
