#include "Lexer.h"
#include "Error.h"
#include <ctype.h>
#include <string>

namespace lua
{
#define THROW_LEX_ERROR(src, desc)                                  \
    Error::ThrowError(                                              \
            src->GetLineNum(),                                      \
            src->GetColumnNum(),                                    \
            desc)

    Lexer::Lexer(Source *source, LexTable *lex_table, State *state)
        : state_(state),
          source_(source),
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
                    THROW_LEX_ERROR(source_, "expect '=' here to construct '~='");
            default:
                return LexKeyWordAndIdentifier();
            }
        }

        return -1;
    }

    void LexHexBaseNumber(Source *source, std::string& num_str)
    {
        if (num_str.size() != 1 || num_str[0] != '0')
            THROW_LEX_ERROR(source, "expect '0x' or '0X' here to construct hex number");

        // Push 'x' or 'X'
        num_str.push_back(source->Next());
        int c = source->Peek();
        while (c != -1)
        {
            if (isdigit(c))
                num_str.push_back(source->Next());
            else if (isalpha(c))
                THROW_LEX_ERROR(source, "unexpect character after number");
            else
                break;
            c = source->Peek();
        }

        // If num_str value is '0x'
        if (num_str.size() <= 2)
            THROW_LEX_ERROR(source, "incomplete hex number here");
    }

    bool LexFractionalPart(Source *source, std::string& num_str)
    {
        // Push '.'
        num_str.push_back(source->Next());

        int c = source->Peek();
        while (c != -1)
        {
            if (isdigit(c))
                num_str.push_back(source->Next());
            else
                break;
            c = source->Peek();
        }

        return true;
    }

    bool LexDecimalPart(Source *source, std::string& num_str)
    {
        num_str.push_back(source->Next());

        int c = source->Peek();
        if (c == '-' || c == '+')
            num_str.push_back(source->Next());

        c = source->Peek();
        while (c != -1)
        {
            if (isdigit(c))
                num_str.push_back(source->Next());
            else
                break;
            c = source->Peek();
        }

        // If the last char is not digit, then it can not construct a number.
        if (!isdigit(num_str.back()))
            THROW_LEX_ERROR(source, "incomplete decimal part of number");
        return true;
    }

    void LexDecimalBaseNumber(Source *source, std::string& num_str, bool has_fractional)
    {
        bool has_decimal = false;
        int c = source->Peek();

        while (c != -1)
        {
            if (isdigit(c))
                num_str.push_back(source->Next());
            else if (c == '.' && !has_fractional && !has_decimal)
                has_fractional = LexFractionalPart(source, num_str);
            else if (!has_decimal && (c == 'e' || c == 'E'))
                has_decimal = LexDecimalPart(source, num_str);
            else if (isalpha(c) || c == '.')
                THROW_LEX_ERROR(source, "unexpect character after number");
            else
                break;
            c = source->Peek();
        }
    }

    int Lexer::LexNumber()
    {
        std::string num_str;
        num_str.push_back(source_->Next());

        int c = source_->Peek();
        if (c == 'x' || c == 'X')
            LexHexBaseNumber(source_, num_str);
        else
            LexDecimalBaseNumber(source_, num_str, false);

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
                THROW_LEX_ERROR(source_, "expect '\"' here to complete string");
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
        while (c != Source::EOS)
        {
            if (c == ']')
            {
                int oc = source_->Next();
                c = source_->Peek();
                if (c == ']')
                    break;
                long_str.push_back(oc);
            }
            else
            {
                long_str.push_back(source_->Next());
                c = source_->Peek();
            }
        }

        if (c == Source::EOS)
            THROW_LEX_ERROR(source_, "expect ']]' here to complete string");

        source_->Next(); // this character must be ']'
        return lex_table_->InsertNewToken(long_str, STRING);
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
        else if (isdigit(c))
        {
            std::string str_num;
            str_num.push_back('.');
            LexDecimalBaseNumber(source_, str_num, true);
            return lex_table_->InsertNewToken(str_num, NUMBER);
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
            THROW_LEX_ERROR(source_, "unexpect illegal character here");

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
        THROW_LEX_ERROR(source_, "unfinished long commment here");
    }
} // namespace lua
