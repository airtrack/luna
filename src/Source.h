#ifndef SOURCE_H
#define SOURCE_H

#include <vector>
#include <stdio.h>

namespace lua
{
    class Source
    {
    public:
        enum
        {
            EOS = -1,
        };

        explicit Source(FILE *file);

        int Peek() const;
        int Next();
        void Back();

        int GetLineNum() const
        {
            return cur_line_number_;
        }

        int GetColumnNum() const
        {
            return cur_column_number_;
        }

    private:
        std::vector<unsigned char> source_buf_;
        int cur_pos_;
        int cur_line_number_;
        int cur_column_number_;
    };
} // namespace lua

#endif // SOURCE_H
