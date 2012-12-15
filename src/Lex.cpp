#include "Lex.h"
#include "Exception.h"
#include <stddef.h>

namespace luna
{
    Lexer::Lexer(CharInStream in)
        : in_stream_(in),
          current_(EOF),
          line_(1),
          column_(0)
    {
    }

    int Lexer::GetToken(TokenDetail *detail)
    {
        if (current_ == EOF)
            current_ = Next();

        while (current_ != EOF)
        {
            switch (current_) {
            case ' ': case '\t': case '\v':
                current_ = Next();
                break;
            case '\r': case '\n':
                LexNewLine();
                break;
            case '-':
                {
                    int next = Next();
                    if (next == '-')
                        LexComment();
                    else
                    {
                        current_ = next;
                        return '-';
                    }
                }
                break;
            }
        }

        return Token_EOF;
    }

    void Lexer::LexNewLine()
    {
        int next = Next();
        if ((next == '\r' || next == '\n') && next != current_)
            current_ = Next();
        else
            current_ = next;
        ++line_;
        column_ = 0;
    }

    void Lexer::LexComment()
    {
        current_ = Next();
        if (current_ == '[')
            LexMultiLineComment();
        else
            LexSingleLineComment();
    }

    void Lexer::LexMultiLineComment()
    {
        int bracket = 1;
        current_ = Next();
        if (current_ == '[')
        {
            bracket = 2;
            current_ = Next();
        }

        bool is_comment_end = false;
        while (!is_comment_end)
        {
            if (current_ == '-')
            {
                current_ = Next();
                if (current_ == '-')
                {
                    int i = 0;
                    for (; i < bracket; ++i)
                    {
                        current_ = Next();
                        if (current_ != ']')
                            break;
                    }

                    if (i == bracket)
                    {
                        is_comment_end = true;
                        current_ = Next();
                    }
                }
            }
            else if (current_ == EOF)
            {
                // uncompleted multi-line comment
                throw LexException("expect complete multi-line comment before <eof>.");
            }
            else if (current_ == '\r' || current_ == '\n')
            {
                LexNewLine();
            }
            else
            {
                current_ = Next();
            }
        }
    }

    void Lexer::LexSingleLineComment()
    {
        while (current_ != '\r' && current_ != '\n' && current_ != EOF)
            current_ = Next();
    }
} // namespace luna
