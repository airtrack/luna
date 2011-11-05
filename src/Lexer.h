#ifndef LEXER_H
#define LEXER_H

#include "LexTable.h"
#include "Source.h"

namespace lua
{
    class Lexer
    {
    public:
        static void Lex(Source *source, LexTable *lex_table);
    };
} // namespace lua

#endif // LEXER_H
