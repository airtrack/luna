#ifndef LEXER_H
#define LEXER_H

#include "Source.h"
#include "LexTable.h"
#include <stack>

namespace lua
{
    class State;
    class NameSet;
    class UpValueNameSet;

    class Lexer
    {
    public:
        Lexer(Source *source, LexTable *lex_table, State *state);

        State * GetState() const
        {
            return state_;
        }

        void SetFuncStartLevel(std::size_t level)
        {
            func_start_level_ = level;
        }

        std::size_t GetFuncStartLevel() const
        {
            return func_start_level_;
        }

        NameSet * GetLocalNameSet() const
        {
            return local_set_;
        }

        void SetLocalNameSet(NameSet *set)
        {
            local_set_ = set;
        }

        UpValueNameSet * GetUpValueNameSet() const
        {
            return up_value_set_;
        }

        void SetUpValueNameSet(UpValueNameSet *set)
        {
            up_value_set_ = set;
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

        NameSet *local_set_;
        UpValueNameSet *up_value_set_;
        std::size_t func_start_level_;
    };

    class LocalNameSetter
    {
    public:
        LocalNameSetter(Lexer *lexer, NameSet *new_name_set)
            : lexer_(lexer),
              old_name_set_(lexer->GetLocalNameSet())
        {
            lexer_->SetLocalNameSet(new_name_set);
        }

        ~LocalNameSetter()
        {
            lexer_->SetLocalNameSet(old_name_set_);
        }

    private:
        Lexer *lexer_;
        NameSet *old_name_set_;
    };

    class UpValueNameSetter
    {
    public:
        UpValueNameSetter(Lexer *lexer, UpValueNameSet *new_name_set)
            : lexer_(lexer),
              old_name_set_(lexer->GetUpValueNameSet())
        {
            lexer_->SetUpValueNameSet(new_name_set);
        }

        ~UpValueNameSetter()
        {
            lexer_->SetUpValueNameSet(old_name_set_);
        }

    private:
        Lexer *lexer_;
        UpValueNameSet *old_name_set_;
    };

    class FuncStartLevelSetter
    {
    public:
        FuncStartLevelSetter(Lexer *lexer, std::size_t new_func_start_level)
            : lexer_(lexer),
              old_func_start_level_(lexer->GetFuncStartLevel())
        {
            lexer_->SetFuncStartLevel(new_func_start_level);
        }

        ~FuncStartLevelSetter()
        {
            lexer_->SetFuncStartLevel(old_func_start_level_);
        }

    private:
        Lexer *lexer_;
        std::size_t old_func_start_level_;
    };
} // namespace lua

#endif // LEXER_H
