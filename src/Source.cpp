#include "Source.h"
#include "Error.h"
#include "io/IFileStream.h"
#include "io/ICacheStream.h"
#include "io/ITextStream.h"
#include "io/ICRLFFilterStream.h"

namespace lua
{
    Source::Source(const char *file)
        : cur_line_number_(1),
          cur_column_number_(0)
    {
        std::unique_ptr<io::IFileStream> src(new io::IFileStream(file));
        if (!src->IsOpen())
            throw OpenFileError(file);

        src_stream_ = io::IStreamPtr(new io::ICacheStream(std::move(src)));
        src_stream_ = io::IStreamPtr(new io::IMBSTextStream(std::move(src_stream_)));
        src_stream_ = io::IStreamPtr(new io::ICRLFFilterStream(std::move(src_stream_)));
    }

    int Source::Peek() const
    {
        int c = src_stream_->Peek();
        if (c == EOF)
            return EOS;
        return c;
    }

    int Source::Next()
    {
        int c = src_stream_->Get();
        if (c == EOF)
            return EOS;

        if (c == '\n' || c == '\r')
        {
            ++cur_line_number_;
            cur_column_number_ = 0;
        }
        else
        {
            ++cur_column_number_;
        }

        return c;
    }
} // namespace lua
