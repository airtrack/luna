#include "Lexer.h"

namespace lua
{
    Lexer::Lexer(Source *source, LexTable *lex_table, ErrorList *error_list)
        : source_(source),
          lex_table_(lex_table),
          error_list_(error_list)
    {
    }

    int Lexer::GetToken()
    {

    }
} // namespace lua
