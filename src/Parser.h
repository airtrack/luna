#ifndef PARSER_H
#define PARSER_H

#include "Lex.h"

namespace luna
{
    class Parser
    {
    public:
        Parser() { }

        Parser(const Parser&) = delete;
        void operator = (const Parser&) = delete;

        bool Parse(Lexer *lexer) { return false; }
    };
} // namespace luna

#endif // PARSER_H
