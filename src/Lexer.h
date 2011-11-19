#ifndef LEXER_H
#define LEXER_H

#include "Source.h"
#include "LexTable.h"

namespace lua
{
    class Lexer
    {
    public:
        Lexer(Source *source, LexTable *lex_table);

        // Return token storing LexTable
        LexTable * GetLexTable() const
        {
            return lex_table_;
        }

        // Return token index of LexTable
        // if return value is -1, then the source is parse over.
        int GetToken();

    private:
        int LexToken();
        int LexNumber();
        int LexString();
        int LexLongString();
        int LexUniqueOperator();
        int LexKeyWordAndIdentifier();
        int LexOperatorAndNext(const char *op, TokenType type);
        void SkipWhiteChar();
        void LexComment();
        void LexSingleLineComment();
        void LexMultiLineComment();

        Source *source_;
        LexTable *lex_table_;
    };
} // namespace lua

#endif // LEXER_H
