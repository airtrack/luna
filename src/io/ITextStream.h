#ifndef ITEXT_STREAM_H
#define ITEXT_STREAM_H

#include "IStream.h"
#include <memory>

namespace io
{
    // Multi-byte string text input stream class.
    // This class maybe convert string encode, so Seek and Tell function are
    // uncallable.
    class IMBSTextStream : public IStream
    {
    public:
        class StringConverter;

        explicit IMBSTextStream(IStreamPtr &&istream);

        virtual int Get();
        virtual int Peek();
        virtual std::size_t Read(char *buf, std::size_t buf_len);
        virtual void Unget(int c);
        virtual void Seek(int off, SeekType seek_type);
        virtual std::size_t Tell();

    private:
        std::unique_ptr<StringConverter> converter_;
    };
} // namespace io

#endif // ITEXT_STREAM_H
