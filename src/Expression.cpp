#include "Expression.h"

namespace lua
{
    bool BasicExpression::ParseNode(Lexer *lexer)
    {
        return true;
    }

    bool NameListExpression::ParseNode(Lexer *lexer)
    {
        return true;
    }

    int NameListExpression::GetNameCount() const
    {
        return 0;
    }

    bool ExpListExpression::ParseNode(Lexer *lexer)
    {
        return true;
    }

    int ExpListExpression::GetExpCount() const
    {
        return 0;
    }

    bool FuncNameExpression::ParseNode(Lexer *lexer)
    {
        return true;
    }

    bool ParamListExpression::ParseNode(Lexer *lexer)
    {
        return true;
    }
} // namespace lua
