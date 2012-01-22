#include "Error.h"
#include <sstream>

namespace lua
{
    // static
    void Error::ThrowError(int line, int column, const std::string& desc)
    {
        throw Error(line, column, desc);
    }

    std::string Error::ConvertToReadable(const Error& error)
    {
        std::ostringstream es;
        es << "Error [line:" << error.error_line
            << "][column:" << error.error_column << "]:" << error.desc_helper;
        return es.str();
    }
} // namespace lua
