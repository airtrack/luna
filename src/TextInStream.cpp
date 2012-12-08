#include "TextInStream.h"

namespace io {
namespace text {

	InStream::InStream(const std::string& path)
		: stream_(nullptr)
	{
		stream_ = fopen(path.c_str(), "rb");
	}

	InStream::~InStream()
	{
		if (stream_)
			fclose(stream_);
	}

} // namespace text
} // namespace io
