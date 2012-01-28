#include "IFileStream.h"

namespace io
{
    IFileStream::IFileStream(const char *file)
        : file_(0)
    {
#ifdef _MSC_VER
        fopen_s(&file_, file, "rb");
#else
        file_ = fopen(file, "rb");
#endif
    }

    bool IFileStream::IsOpen() const
    {
        return file_ != 0;
    }

    int IFileStream::Get()
    {
        return fgetc(file_);
    }

    int IFileStream::Peek()
    {
        int c = fgetc(file_);
        if (c != EOF)
            ungetc(c, file_);
        return c;
    }

    std::size_t IFileStream::Read(char *buf, std::size_t buf_len)
    {
        return fread(buf, 1, buf_len, file_);
    }

    void IFileStream::Unget(int c)
    {
        ungetc(c, file_);
    }

    void IFileStream::Seek(int off, SeekType seek_type)
    {
        fseek(file_, off, seek_type);
    }

    std::size_t IFileStream::Tell()
    {
        return ftell(file_);
    }
} // namespace io
