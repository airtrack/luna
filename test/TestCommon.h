#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "Lex.h"
#include "Parser.h"
#include "State.h"
#include "String.h"
#include "TextInStream.h"
#include "Exception.h"
#include <functional>

class ParserWrapper
{
public:
    explicit ParserWrapper(const std::string &str)
        : iss_(str), state_(), name_("parser"),
          lexer_(&state_, &name_, std::bind(&io::text::InStringStream::GetChar, &iss_)), parser_(&state_)
    {
    }

    bool IsEOF()
    {
        luna::TokenDetail detail;
        return lexer_.GetToken(&detail) == luna::Token_EOF;
    }

    std::unique_ptr<luna::SyntaxTree> Parse()
    {
        return parser_.Parse(&lexer_);
    }

private:
    io::text::InStringStream iss_;
    luna::State state_;
    luna::String name_;
    luna::Lexer lexer_;
    luna::Parser parser_;
};

#endif // TEST_COMMON_H
