#ifndef ICACHE_STREAM_H
#define ICACHE_STREAM_H

#include "IStream.h"

namespace io
{
    // This cache stream will cache all content in memory.
    // so if the content is very big, do not use this cache stream class.
    class ICacheStream : public IStream
    {
    public:
        explicit ICacheStream(IStreamPtr &&istream);
        virtual ~ICacheStream();

        virtual int Get();
        virtual int Peek();
        virtual std::size_t Read(char *buf, std::size_t buf_len);
        virtual void Unget(int c);
        virtual void Seek(int off, SeekType seek_type);
        virtual std::size_t Tell();

    private:
        IStreamPtr istream_;
        char *cache_;
        std::size_t cache_len_;
        std::size_t pos_;
    };
} // namespace io

#endif // ICACHE_STREAM_H
