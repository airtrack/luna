#include "ErrorList.h"

namespace lua
{
    void ErrorList::AddError(ErrorType type, int line, int column, const std::string& desc)
    {
        Error *err = new Error;
        err->type = type;
        err->error_line = line;
        err->error_column = column;
        err->desc_helper = desc;
        list_.push_back(err);
    }
} // namespace lua