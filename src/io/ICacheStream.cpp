#include "ICacheStream.h"
#include <assert.h>
#include <string.h>
#include <algorithm>

namespace io
{
    ICacheStream::ICacheStream(IStreamPtr &&istream)
        : istream_(std::move(istream)),
          cache_(0),
          cache_len_(0),
          pos_(0)
    {
        std::size_t cur = istream_->Tell();
        istream_->Seek(0, SEEK_FROM_END);
        std::size_t len = istream_->Tell() - cur;
        istream_->Seek(cur, SEEK_FROM_BEGIN);

        cache_ = new char[len];
        cache_len_ = istream_->Read(cache_, len);
    }

    ICacheStream::~ICacheStream()
    {
        delete [] cache_;
    }

    int ICacheStream::Get()
    {
        if (pos_ < cache_len_)
            return static_cast<unsigned char>(cache_[pos_++]);
        return EOF;
    }

    int ICacheStream::Peek()
    {
        if (pos_ < cache_len_)
            return static_cast<unsigned char>(cache_[pos_]);
        return EOF;
    }

    std::size_t ICacheStream::Read(char *buf, std::size_t buf_len)
    {
        std::size_t read_len = std::min(cache_len_ - pos_, buf_len);
        if (read_len > 0)
        {
            memcpy(buf, &cache_[pos_], read_len);
            pos_ += read_len;
        }

        return read_len;
    }

    void ICacheStream::Unget(int c)
    {
        if (pos_ > 0)
            cache_[--pos_] = c;
    }

    void ICacheStream::Seek(int off, SeekType seek_type)
    {
        switch (seek_type)
        {
        case SEEK_CURRENT:
            if (off >= 0)
                pos_ += off;
            else
                pos_ -= (-off);
            break;
        case SEEK_FROM_BEGIN:
            assert(off >= 0);
            pos_ = off;
            break;
        case SEEK_FROM_END:
            assert(off <= 0);
            pos_ = cache_len_ - (-off);
            break;
        }
    }

    std::size_t ICacheStream::Tell()
    {
        return pos_;
    }
} // namespace io
