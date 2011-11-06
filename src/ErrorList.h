#ifndef ERROR_LIST_H
#define ERROR_LIST_H

#include <vector>
#include <string>

namespace lua
{
    class ErrorList
    {
    public:
        enum ErrorType
        {

        };

        struct Error
        {
            ErrorType type;
            int error_line;
            int error_column;
            std::string desc_helper;
        };

        void AddError(ErrorType type, int line, int column, const std::string& desc);

    private:
        typedef std::vector<Error *> EList;
        EList list_;
    };
} // namespace lua

#endif // ERROR_LIST_H
