#include "Parser.h"
#include "Statement.h"
#include "NameSet.h"

namespace lua
{
    Parser::Parser(Source *source, State *state)
        : lexer_(source, &lex_table_, state)
    {
    }

    ParseTreeNodePtr Parser::Parse()
    {
        NameSet local_name_set;
        std::unique_ptr<NameSet> up_value_set(new NameSet);
        LocalNameSetter local_setter(&lexer_, &local_name_set);
        UpValueNameSetter up_value_setter(&lexer_, up_value_set.get());

        StatementPtr block_stmt = ParseChunkStatement(&lexer_);
        return StatementPtr(new FunctionStatement(NO_FUNC_NAME,
            ExpressionPtr(), ExpressionPtr(), std::move(block_stmt), std::move(up_value_set)));
    }
} // namespace lua
