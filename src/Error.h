#ifndef ERROR_LIST_H
#define ERROR_LIST_H

#include <vector>
#include <string>

namespace lua
{
    struct Error
    {
        enum Type
        {
            NO_MULTILINE_COMMENT_ENDER,
            NO_STRING_ENDER,
            NO_LONG_STRING_ENDER,
            NO_COMPLETE_NOT_EQUAL,
            INVALIDATE_NUMBER,
            ILLEGAL_CHARACTER,
        };

        Error(Type t, int line, int column, const std::string& desc)
            : type(t), error_line(line), error_column(column), desc_helper(desc)
        {
        }

        Type type;
        int error_line;
        int error_column;
        std::string desc_helper;

        static void ThrowError(Type type, int line, int column, const std::string& desc);
    };
} // namespace lua

#endif // ERROR_LIST_H
