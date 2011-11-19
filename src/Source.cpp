#include "Source.h"

namespace lua
{
    Source::Source()
        : cur_pos_(-1),
          cur_line_number_(1),
          cur_column_number_(0)
    {
    }

    int Source::Peek() const
    {
        if (cur_pos_ + 1 < static_cast<int>(source_buf_.size()))
            return source_buf_[cur_pos_ + 1];
        return EOS;
    }

    int Source::Next()
    {
        if (cur_pos_ + 1 < static_cast<int>(source_buf_.size()))
        {
            int c = source_buf_[++cur_pos_];
            if (c == '\n' || c == '\r')
            {
                ++cur_line_number_;
                cur_column_number_ = 0;
            }
            else
            {
                ++cur_column_number_;
            }

            return c;
        }
        return EOS;
    }

    void Source::Back()
    {
        if (cur_pos_ < 0)
            return ;

        int c = source_buf_[cur_pos_];
        // We do not back if current character is whitespace
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            return ;

        --cur_pos_;
        --cur_column_number_;
    }
} // namespace lua
