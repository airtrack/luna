#ifndef TEXT_IN_STREAM_H
#define TEXT_IN_STREAM_H

#include <stdio.h>
#include <string>

namespace io {
namespace text {

    class InStream
    {
    public:
        explicit InStream(const std::string& path);
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

} // namespace text
} // namespace io

#endif // TEXT_IN_STREAM_H
