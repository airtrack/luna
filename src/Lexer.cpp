#include "Lexer.h"
#include "Error.h"
#include <ctype.h>
#include <string>

namespace lua
{
    Lexer::Lexer(Source *source, LexTable *lex_table)
        : source_(source),
          lex_table_(lex_table)
    {
        KeyWordSet::InitKeyWordSet();
    }

    int Lexer::GetToken()
    {
        if (unget_.empty())
            return LexToken();

        int token = unget_.top();
        unget_.pop();
        return token;
    }

    void Lexer::UngetToken(int token)
    {
        unget_.push(token);
    }

    int Lexer::LexToken()
    {
        while (true)
        {
            SkipWhiteChar();
            int c = source_->Peek();
            if (c == Source::EOS)
                return -1;

            switch (c)
            {
            case '-':
                source_->Next();
                c = source_->Peek();
                if (c == '-')
                    LexComment();
                else
                    return lex_table_->InsertNewToken("-", OP_MINUS);
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return LexNumber();
            case '\"': case '\'':
                return LexString();
            case '[':
                source_->Next();
                c = source_->Peek();
                if (c == '[')
                    return LexLongString();
                else
                    return lex_table_->InsertNewToken("[", OP_LEFT_BRACKET);
            case '+': case '*': case '/': case '%': case '^':
            case '(': case ')': case ',': case ';': case '#':
            case '{': case '}': case ']': case ':':
                return LexUniqueOperator();
            case '.':
                return LexDotsOperator();
            case '=':
                source_->Next();
                c = source_->Peek();
                if (c == '=')
                    return LexOperatorAndNext("==", OP_EQUAL);
                else
                    return lex_table_->InsertNewToken("=", OP_ASSIGN);
            case '<':
                source_->Next();
                c = source_->Peek();
                if (c == '=')
                    return LexOperatorAndNext("<=", OP_LESSEQUAL);
                else
                    return lex_table_->InsertNewToken("<", OP_LESS);
            case '>':
                source_->Next();
                c = source_->Peek();
                if (c == '=')
                    return LexOperatorAndNext(">=", OP_GREATEREQUAL);
                else
                    return lex_table_->InsertNewToken(">", OP_GREATER);
            case '~':
                source_->Next();
                c = source_->Peek();
                if (c == '=')
                    return LexOperatorAndNext("~=", OP_NOTEQUAL);
                else
                {
                    LexError::ThrowError(LexError::NO_COMPLETE_NOT_EQUAL_OP,
                        source_->GetLineNum(), source_->GetColumnNum(), "~=");
                    return -1;
                }
            default:
                return LexKeyWordAndIdentifier();
            }
        }

        return -1;
    }

    int Lexer::LexNumber()
    {
        std::string num_str;
        bool has_decimal_point = false;
        while (true)
        {
            int c = source_->Peek();
            if (isdigit(c))
                num_str.push_back(c);
            else if (!has_decimal_point && c == '.')
            {
                num_str.push_back(c);
                source_->Next();
                c = source_->Peek();
                if (c == '.')
                {
                    // Next is ".." operator, so we back
                    source_->Back();
                    num_str.pop_back();
                    break;
                }
                has_decimal_point = true;
            }
            else if (isalpha(c))
            {
                LexError::ThrowError(LexError::INVALIDATE_NUMBER,
                    source_->GetLineNum(), source_->GetColumnNum(), num_str);
                return -1;
            }
            else
                break;

            source_->Next();
        }

        return lex_table_->InsertNewToken(num_str, NUMBER);
    }

    int Lexer::LexString()
    {
        // starter character is same with ender
        int ender = source_->Next();
        std::string str;

        while (true)
        {
            int c = source_->Peek();
            if (c == Source::EOS ||
                c == '\n' || c == '\r')
            {
                std::string err_helper;
                err_helper.push_back(ender);
                LexError::ThrowError(LexError::NO_STRING_ENDER,
                    source_->GetLineNum(), source_->GetColumnNum(), err_helper);
                return -1;
            }
            else if (c == ender)
            {
                source_->Next();
                return lex_table_->InsertNewToken(str, STRING);
            }
            else
            {
                str.push_back(c);
            }
            source_->Next();
        }

        return -1;
    }

    int Lexer::LexLongString()
    {
        std::string long_str;
        source_->Next();

        int c = source_->Peek();
        while (c != Source::EOS && c != '[' && c != ']')
        {
            long_str.push_back(source_->Next());
            c = source_->Peek();
        }

        if (c == Source::EOS || c == '[')
        {
            LexError::ThrowError(LexError::NO_LONG_STRING_ENDER,
                source_->GetLineNum(), source_->GetColumnNum(), "]]");
            return -1;
        }

        source_->Next(); // this character must be ']'
        if (source_->Peek() == ']')
        {
            // complete long string
            source_->Next();
            return lex_table_->InsertNewToken(long_str, STRING);
        }

        LexError::ThrowError(LexError::NO_LONG_STRING_ENDER,
            source_->GetLineNum(), source_->GetColumnNum(), "]]");
        return -1;
    }

    int Lexer::LexUniqueOperator()
    {
        int c = source_->Next();
        std::string op;
        op.push_back(c);

        TokenType type = OPERATOR;
        switch (c)
        {
        case '+': type = OP_PLUS; break;
        case '*': type = OP_MULTIPLY; break;
        case '/': type = OP_DIVIDE; break;
        case '%': type = OP_MOD; break;
        case '^': type = OP_POWER; break;
        case '(': type = OP_LEFT_PARENTHESE; break;
        case ')': type = OP_RIGHT_PARENTHESE; break;
        case ',': type = OP_COMMA; break;
        case ';': type = OP_SEMICOLON; break;
        case '#': type = OP_POUND; break;
        case '{': type = OP_LEFT_BRACE; break;
        case '}': type = OP_RIGHT_BRACE; break;
        case ']': type = OP_RIGHT_BRACKET; break;
        case ':': type = OP_COLON; break;
        }
        return lex_table_->InsertNewToken(op, type);
    }

    int Lexer::LexDotsOperator()
    {
        source_->Next();
        int c = source_->Peek();

        if (c == '.')
        {
            source_->Next();
            c = source_->Peek();
            if (c == '.')
                return LexOperatorAndNext("...", OP_PARAM_LIST);
            else
                return lex_table_->InsertNewToken("..", OP_CONCAT);
        }
        else
        {
            return lex_table_->InsertNewToken(".", OP_DOT);
        }
    }

    int Lexer::LexKeyWordAndIdentifier()
    {
        std::string identifier;

        while (true)
        {
            int c = source_->Peek();
            if (isalnum(c) || c == '_')
                identifier.push_back(c);
            else
                break;
            source_->Next();
        }

        if (identifier.empty())
        {
            LexError::ThrowError(LexError::ILLEGAL_CHARACTER,
                source_->GetLineNum(), source_->GetColumnNum(), "");
            return -1;
        }

        TokenType type = KeyWordSet::GetTokenType(identifier);
        return lex_table_->InsertNewToken(identifier, type);
    }

    int Lexer::LexOperatorAndNext(const char *op, TokenType type)
    {
        source_->Next();
        return lex_table_->InsertNewToken(op, type);
    }

    void Lexer::SkipWhiteChar()
    {
        while (true)
        {
            int c = source_->Peek();
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                source_->Next();
            else
                break;
        }
    }

    void Lexer::LexComment()
    {
        source_->Next();
        if (source_->Peek() == '[')
        {
            source_->Next();
            if (source_->Peek() == '[')
                LexMultiLineComment();
            else
                LexSingleLineComment();
        }
        else
        {
            LexSingleLineComment();
        }
    }

    void Lexer::LexSingleLineComment()
    {
        while (source_->Peek() != Source::EOS)
        {
            int c = source_->Next();
            if (c == '\n' || c == '\r')
                return ;
        }
    }

    void Lexer::LexMultiLineComment()
    {
        source_->Next();
        while (source_->Peek() != Source::EOS)
        {
            if (source_->Next() == '-' &&
                source_->Next() == '-' &&
                source_->Next() == ']' &&
                source_->Next() == ']')
                return ;
        }

        // Not found multi-line comment ender, so there has an error
        LexError::ThrowError(LexError::NO_MULTILINE_COMMENT_ENDER,
            source_->GetLineNum(), source_->GetColumnNum(), "--]]");
    }
} // namespace lua
