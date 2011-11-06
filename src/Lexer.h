#ifndef LEXER_H
#define LEXER_H

#include "Source.h"
#include "LexTable.h"
#include "ErrorList.h"

namespace lua
{
    class Lexer
    {
    public:
        Lexer(Source *source, LexTable *lex_table, ErrorList *error_list);

        // Return token storing LexTable
        LexTable * GetLexTable() const
        {
            return lex_table_;
        }

        // Return token index of LexTable
        // if return value is -1, then the source is parse over.
        int GetToken();

    private:
        Source *source_;
        LexTable *lex_table_;
        ErrorList *error_list_;
    };
} // namespace lua

#endif // LEXER_H
