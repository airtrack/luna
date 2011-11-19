#include "Error.h"

namespace lua
{
    // static
    void Error::ThrowError(Type type, int line, int column, const std::string& desc)
    {
        throw Error(type, line, column, desc);
    }
} // namespace lua