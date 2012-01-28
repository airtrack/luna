#ifndef ICRLF_FILTER_STREAM_H
#define ICRLF_FILTER_STREAM_H

#include "IStream.h"

namespace io
{
    // This class will filter all "\r\n" from the stream, replace by '\n'.
    class ICRLFFilterStream : public IStream
    {
    public:
        explicit ICRLFFilterStream(IStreamPtr &&istream);

        virtual int Get();
        virtual int Peek();
        virtual std::size_t Read(char *buf, std::size_t buf_len);
        virtual void Unget(int c);
        virtual void Seek(int off, SeekType seek_type);
        virtual std::size_t Tell();

    private:
        IStreamPtr istream_;
    };
} // namespace io

#endif // ICRLF_FILTER_STREAM_H
