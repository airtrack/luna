#ifndef BUILDIN_FUNCTIONS_H
#define BUILDIN_FUNCTIONS_H

#include "NativeFunctionStackOperator.h"

namespace buildin
{
    void RegisterBuildin(lua::State *state);
} // namespace buildin

#endif // BUILDIN_FUNCTIONS_H
