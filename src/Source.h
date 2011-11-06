#ifndef SOURCE_H
#define SOURCE_H

#include <vector>

namespace lua
{
    class Source
    {
    public:
        Source();

        char Peek() const;
        char Next();
        void Backspace(std::size_t count = 1);

    private:
        std::vector<char> source_buf_;
        int cur_pos_;
        int cur_line_number_;
        int cur_column_number_;
    };
} // namespace lua

#endif // SOURCE_H
