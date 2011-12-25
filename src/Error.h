#ifndef ERROR_LIST_H
#define ERROR_LIST_H

#include <vector>
#include <string>

namespace lua
{
    struct LexError
    {
        enum Type
        {
            NO_MULTILINE_COMMENT_ENDER,
            NO_STRING_ENDER,
            NO_LONG_STRING_ENDER,
            NO_COMPLETE_NOT_EQUAL_OP,
            INVALIDATE_NUMBER,
            ILLEGAL_CHARACTER,
        };

        LexError(Type t, int line, int column, const std::string& desc)
            : type(t), error_line(line), error_column(column), desc_helper(desc)
        {
        }

        Type type;
        int error_line;
        int error_column;
        std::string desc_helper;

        static void ThrowError(Type type, int line, int column, const std::string& desc);
        static std::string ConvertToReadable(const LexError& error);
    };

    struct ParserError
    {
        int error_line;
        int error_column;
        std::string desc_helper;

        ParserError(int line, int column, const std::string& desc)
            : error_line(line),
              error_column(column),
              desc_helper(desc)
        {
        }

        static void ThrowError(int line, int column, const std::string& desc);
    };
} // namespace lua

#endif // ERROR_LIST_H
