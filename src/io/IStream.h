#ifndef ISTREAM_H
#define ISTREAM_H

#include <stdio.h>
#include <memory>

namespace io
{
    enum SeekType
    {
        SEEK_CURRENT = SEEK_CUR,
        SEEK_FROM_BEGIN = SEEK_SET,
        SEEK_FROM_END = SEEK_END,
    };

    class IStream
    {
    public:
        virtual ~IStream() { }

        virtual int Get() = 0;
        virtual int Peek() = 0;
        virtual std::size_t Read(char *buf, std::size_t buf_len) = 0;
        virtual void Unget(int c) = 0;
        virtual void Seek(int off, SeekType seek_type) = 0;
        virtual std::size_t Tell() = 0;
    };

    typedef std::unique_ptr<IStream> IStreamPtr;
} // namespace io

#endif // ISTREAM_H
