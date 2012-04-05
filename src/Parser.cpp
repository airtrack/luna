#include "Parser.h"
#include "Statement.h"
#include "State.h"
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
        // Push first level as global
        NameSetLevelPusher level_pusher(&local_name_set);

        // New up value set for chunk
        std::unique_ptr<UpValueNameSet> up_value_set(new UpValueNameSet);

        LocalNameSetter local_setter(&lexer_, &local_name_set);
        UpValueNameSetter up_value_setter(&lexer_, up_value_set.get());

        StatementPtr block_stmt = ParseChunkStatement(&lexer_);
        Function *func = lexer_.GetState()->GetDataPool()->GetFunction(std::move(up_value_set));

        return StatementPtr(new FunctionStatement(NO_FUNC_NAME, 0,
            ExpressionPtr(), ExpressionPtr(), std::move(block_stmt), func));
    }
} // namespace lua
