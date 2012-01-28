#ifndef IFILE_STREAM_H
#define IFILE_STREAM_H

#include "IStream.h"

namespace io
{
    // This class open file as stream to read.
    class IFileStream : public IStream
    {
    public:
        explicit IFileStream(const char *file);

        bool IsOpen() const;

        virtual int Get();
        virtual int Peek();
        virtual std::size_t Read(char *buf, std::size_t buf_len);
        virtual void Unget(int c);
        virtual void Seek(int off, SeekType seek_type);
        virtual std::size_t Tell();

    private:
        FILE *file_;
    };
} // namespace io

#endif // IFILE_STREAM_H
