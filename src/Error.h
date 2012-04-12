#ifndef ERROR_LIST_H
#define ERROR_LIST_H

#include <vector>
#include <string>

namespace lua
{
    struct OpenFileError
    {
        std::string file;

        explicit OpenFileError(const std::string& f) : file(f) { }
    };

    struct Error
    {
        int error_line;
        int error_column;
        std::string desc_helper;

        Error(int line, int column, const std::string& desc)
            : error_line(line),
              error_column(column),
              desc_helper(desc)
        {
        }

        static void ThrowError(int line, int column, const std::string& desc);
        static std::string ConvertToReadable(const Error& error);
    };

    struct RuntimeError
    {
        std::string info;

        explicit RuntimeError(const std::string& i) : info(i) { }
    };
} // namespace lua

#endif // ERROR_LIST_H
