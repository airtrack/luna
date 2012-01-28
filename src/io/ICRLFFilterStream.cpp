#include "ICRLFFilterStream.h"

namespace io
{
    ICRLFFilterStream::ICRLFFilterStream(IStreamPtr &&istream)
        : istream_(std::move(istream))
    {
    }

    int ICRLFFilterStream::Get()
    {
        int c = istream_->Get();
        if (c == '\r')
        {
            int nc = istream_->Peek();
            if (nc == '\n')
                c = istream_->Get();
        }

        return c;
    }

    int ICRLFFilterStream::Peek()
    {
        int c = Get();
        if (c != EOF)
            Unget(c);
        return c;
    }

    std::size_t ICRLFFilterStream::Read(char *buf, std::size_t buf_len)
    {
        std::size_t count = 0;

        int c = Get();
        while (c != EOF)
        {
            *buf++ = c;

            ++count;
            if (count == buf_len)
                break;

            c = Get();
        }

        return count;
    }

    void ICRLFFilterStream::Unget(int c)
    {
        istream_->Unget(c);
    }

    void ICRLFFilterStream::Seek(int off, SeekType seek_type)
    {
        istream_->Seek(off, seek_type);
    }

    std::size_t ICRLFFilterStream::Tell()
    {
        return istream_->Tell();
    }
} // namespace io
