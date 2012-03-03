#ifndef LEXER_H
#define LEXER_H

#include "Source.h"
#include "LexTable.h"
#include <stack>

namespace lua
{
    class State;

    class Lexer
    {
    public:
        Lexer(Source *source, LexTable *lex_table, State *state);

        State * GetState() const
        {
            return state_;
        }

        // Return token storing LexTable
        LexTable& GetLexTable() const
        {
            return *lex_table_;
        }

        // Return next token index of LexTable
        // if return value is -1, then the source is parse over.
        int GetToken();

        // Back the token, call GetToken function will re-get the token.
        void UngetToken(int token);

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

        std::stack<int> unget_;
        State *state_;
        Source *source_;
        LexTable *lex_table_;
    };
} // namespace lua

#endif // LEXER_H
