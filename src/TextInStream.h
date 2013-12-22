#ifndef TEXT_IN_STREAM_H
#define TEXT_IN_STREAM_H

#include <stdio.h>
#include <string>

namespace io {
namespace text {

    class InStream
    {
    public:
        explicit InStream(const std::string &path);
        ~InStream();

        InStream(const InStream&) = delete;
        void operator = (const InStream&) = delete;

        bool IsOpen() const
        {
            return stream_ != nullptr;
        }

        int GetChar()
        {
            return fgetc(stream_);
        }

    private:
        FILE *stream_;
    };

    class InStringStream
    {
    public:
        explicit InStringStream(const std::string &str);

        InStringStream(const InStringStream&) = delete;
        void operator = (const InStringStream&) = delete;

        void SetInputString(const std::string &input);

        int GetChar()
        {
            if (pos_ < str_.size())
                return static_cast<unsigned char>(str_[pos_++]);
            else
                return EOF;
        }

    private:
        std::string str_;
        std::size_t pos_;
    };

} // namespace text
} // namespace io

#endif // TEXT_IN_STREAM_H
