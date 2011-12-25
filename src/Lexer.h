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

        // Return next token index of LexTable
        // if return value is -1, then the source is parse over.
        int GetToken();

        // Return current token index of LexTable
        // if return value >= 0, then current token is validate,
        // else we need call GetToken() to get new token index.
        int GetCurToken() const;

        // Get current line number of the source
        int GetLineNumber() const
        {
            return source_->GetLineNum();
        }

        // Get current column number of the source
        int GetColumnNumber() const
        {
            return source_->GetColumnNum();
        }

    private:
        int LexToken();
        int LexNumber();
        int LexString();
        int LexLongString();
        int LexUniqueOperator();
        int LexDotsOperator();
        int LexKeyWordAndIdentifier();
        int LexOperatorAndNext(const char *op, TokenType type);
        void SkipWhiteChar();
        void LexComment();
        void LexSingleLineComment();
        void LexMultiLineComment();

        int cur_token_;
        Source *source_;
        LexTable *lex_table_;
    };
} // namespace lua

#endif // LEXER_H
