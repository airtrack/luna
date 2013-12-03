#ifndef CODE_GENERATE_H
#define CODE_GENERATE_H

#include "Visitor.h"
#include <memory>

namespace luna
{
    class State;

    std::unique_ptr<Visitor> GenerateVisitor(State *state);
} // namespace luna

#endif // CODE_GENERATE_H
