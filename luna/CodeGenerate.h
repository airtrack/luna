#ifndef CODE_GENERATE_H
#define CODE_GENERATE_H

#include "Visitor.h"
#include <memory>

namespace luna
{
    class State;

    void CodeGenerate(SyntaxTree *root, State *state);
} // namespace luna

#endif // CODE_GENERATE_H
