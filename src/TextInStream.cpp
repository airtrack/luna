#include "TextInStream.h"

namespace io {
namespace text {
    InStream::InStream(const std::string &path)
        : stream_(nullptr)
    {
        stream_ = fopen(path.c_str(), "rb");
    }

    InStream::~InStream()
    {
        if (stream_)
            fclose(stream_);
    }

    InStringStream::InStringStream(const std::string &str)
        : str_(str),
          pos_(0)
    {
    }

    void InStringStream::SetInputString(const std::string &input)
    {
        str_ = input;
        pos_ = 0;
    }
} // namespace text
} // namespace io
