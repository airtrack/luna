#include "String.h"

namespace luna
{
    String::String()
        : in_heap_(0), str_(nullptr), length_(0), hash_(0)
    {
    }

    String::String(const char *str)
        : String()
    {
        SetValue(str);
    }

    String::~String()
    {
        if (in_heap_)
            delete [] str_;
    }

    std::string String::GetStdString() const
    {
        if (in_heap_)
            return std::string(str_, length_);
        else
            return std::string(str_buffer_, length_);
    }

    void String::SetValue(const std::string &str)
    {
        SetValue(str.c_str(), str.size());
    }

    void String::SetValue(const char *str)
    {
        SetValue(str, strlen(str));
    }

    void String::SetValue(const char *str, std::size_t len)
    {
        if (in_heap_)
            delete [] str_;

        length_ = len;
        if (len < sizeof(str_buffer_))
        {
            memcpy(str_buffer_, str, len);
            str_buffer_[len] = 0;
            in_heap_ = 0;
            Hash(str_buffer_);
        }
        else
        {
            str_ = new char[len + 1];
            memcpy(str_, str, len);
            str_[len] = 0;
            in_heap_ = 1;
            Hash(str_);
        }
    }

    void String::Hash(const char *s)
    {
        hash_ = 5381;
        int c = 0;

        while ((c = *s++))
            hash_ = ((hash_ << 5) + hash_) + c;
    }
} // namespace luna
