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
        explicit Parser(State *state);

        Parser(const Parser&) = delete;
        void operator = (const Parser&) = delete;

        std::unique_ptr<SyntaxTree> Parse(Lexer *lexer);

    private:
        State *state_;
    };
} // namespace luna

#endif // PARSER_H
