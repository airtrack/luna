#include "ITextStream.h"

namespace io
{
    class IMBSTextStream::StringConverter
    {
    public:
        virtual ~StringConverter() { }

        virtual int Get() = 0;
        virtual int Peek() = 0;
        virtual std::size_t Read(char *buf, std::size_t buf_len) = 0;
        virtual void Unget(int c) = 0;
    };

    class DummyConverter : public IMBSTextStream::StringConverter
    {
    public:
        explicit DummyConverter(IStreamPtr &&istream)
            : istream_(std::move(istream))
        {
        }

        virtual int Get()
        {
            return istream_->Get();
        }

        virtual int Peek()
        {
            return istream_->Peek();
        }

        virtual std::size_t Read(char *buf, std::size_t buf_len)
        {
            return istream_->Read(buf, buf_len);
        }

        virtual void Unget(int c)
        {
            istream_->Unget(c);
        }

    private:
        IStreamPtr istream_;
    };

    class UTF16BEConverter : public IMBSTextStream::StringConverter
    {
    public:
        explicit UTF16BEConverter(IStreamPtr &&istream)
            : istream_(std::move(istream))
        {
        }

        virtual int Get()
        {
            return istream_->Get();
        }

        virtual int Peek()
        {
            return istream_->Peek();
        }

        virtual std::size_t Read(char *buf, std::size_t buf_len)
        {
            return istream_->Read(buf, buf_len);
        }

        virtual void Unget(int c)
        {
            istream_->Unget(c);
        }

    private:
        IStreamPtr istream_;
    };

    class UTF16LEConverter : public IMBSTextStream::StringConverter
    {
    public:
        explicit UTF16LEConverter(IStreamPtr &&istream)
            : istream_(std::move(istream))
        {
        }

        virtual int Get()
        {
            return istream_->Get();
        }

        virtual int Peek()
        {
            return istream_->Peek();
        }

        virtual std::size_t Read(char *buf, std::size_t buf_len)
        {
            return istream_->Read(buf, buf_len);
        }

        virtual void Unget(int c)
        {
            istream_->Unget(c);
        }

    private:
        IStreamPtr istream_;
    };

#define STREAM_UNGET(c)             \
    if (c != EOF)                   \
        istream->Unget(c)

    std::unique_ptr<IMBSTextStream::StringConverter> CreateConverter(IStreamPtr &&istream)
    {
        int c1 = istream->Get();
        int c2 = istream->Get();
        int c3 = istream->Get();

        std::unique_ptr<IMBSTextStream::StringConverter> converter;

        if (c1 == 0xEF && c2 == 0xBB && c3 == 0xBF)             // UTF8 BOM
        {
            converter.reset(new DummyConverter(std::move(istream)));
        }
        else if (c1 == 0xFE && c2 == 0xFF)                      // UTF16 big-endian
        {
            STREAM_UNGET(c3);
            converter.reset(new UTF16BEConverter(std::move(istream)));
        }
        else if (c1 == 0xFF && c2 == 0xFE)                      // UTF16 little-endian
        {
            STREAM_UNGET(c3);
            converter.reset(new UTF16LEConverter(std::move(istream)));
        }
        else
        {
            STREAM_UNGET(c3);
            STREAM_UNGET(c2);
            STREAM_UNGET(c1);
            converter.reset(new DummyConverter(std::move(istream)));
        }

        return converter;
    }

    IMBSTextStream::IMBSTextStream(IStreamPtr &&istream)
    {
        converter_ = CreateConverter(std::move(istream));
    }

    int IMBSTextStream::Get()
    {
        return converter_->Get();
    }

    int IMBSTextStream::Peek()
    {
        return converter_->Peek();
    }

    std::size_t IMBSTextStream::Read(char *buf, std::size_t buf_len)
    {
        return converter_->Read(buf, buf_len);
    }

    void IMBSTextStream::Unget(int c)
    {
        return converter_->Unget(c);
    }

    void IMBSTextStream::Seek(int off, SeekType seek_type)
    {
        // Not implement.
    }

    std::size_t IMBSTextStream::Tell()
    {
        // Not implement.
        return 0;
    }
} // namespace io
