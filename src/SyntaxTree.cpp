#include "SyntaxTree.h"
#include "Visitor.h"

namespace luna
{
#define SYNTAX_TREE_ACCEPT_VISITOR_IMPL(class_name)         \
    void class_name::Accept(Visitor *v, void *data)         \
    {                                                       \
        v->Visit(this, data);                               \
    }

    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(Chunk)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(Block)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(ReturnStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(BreakStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(DoStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(WhileStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(RepeatStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(IfStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(ElseIfStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(ElseStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(NumericForStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(GenericForStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(FunctionStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(FunctionName)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(LocalFunctionStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(LocalNameListStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(AssignmentStatement)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(VarList)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(Terminator)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(BinaryExpression)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(UnaryExpression)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(FunctionBody)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(ParamList)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(NameList)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(TableDefine)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(TableIndexField)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(TableNameField)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(TableArrayField)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(IndexAccessor)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(MemberAccessor)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(NormalFuncCall)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(MemberFuncCall)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(FuncCallArgs)
    SYNTAX_TREE_ACCEPT_VISITOR_IMPL(ExpressionList)

} // namespace luna
