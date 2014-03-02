#ifndef PARSER_H
#define PARSER_H

#include "SyntaxTree.h"
#include <memory>

namespace luna
{
    class Lexer;
    class State;

    class Parser
    {
    public:
        Parser() = default;
        Parser(const Parser&) = delete;
        void operator = (const Parser&) = delete;

        std::unique_ptr<SyntaxTree> Parse(Lexer *lexer);
    };
} // namespace luna

#endif // PARSER_H
