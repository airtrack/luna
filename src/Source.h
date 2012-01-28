#ifndef SOURCE_H
#define SOURCE_H

#include "io/IStream.h"

namespace lua
{
    class Source
    {
    public:
        enum
        {
            EOS = -1,
        };

        explicit Source(const char *file);

        int Peek() const;
        int Next();

        int GetLineNum() const
        {
            return cur_line_number_;
        }

        int GetColumnNum() const
        {
            return cur_column_number_;
        }

    private:
        io::IStreamPtr src_stream_;
        int cur_line_number_;
        int cur_column_number_;
    };
} // namespace lua

#endif // SOURCE_H
