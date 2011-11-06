#include "Source.h"

namespace lua
{
    Source::Source()
        : cur_pos_(0),
          cur_line_number_(0),
          cur_column_number_(0)
    {

    }

    char Source::Peek() const
    {
        return source_buf_[cur_pos_];
    }

    char Source::Next()
    {

    }

    void Source::Backspace(std::size_t count)
    {

    }
} // namespace lua
