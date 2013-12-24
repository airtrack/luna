#ifndef STRING_H
#define STRING_H

#include "GC.h"
#include <algorithm>
#include <string>
#include <string.h>

namespace luna
{
    class String : public GCObject
    {
    public:
        String();
        explicit String(const char *str);
        ~String();

        String(const String &) = delete;
        void operator = (const String &) = delete;

        virtual void Accept(GCObjectVisitor *v)
        { v->Visit(this); }

        std::size_t GetHash() const
        { return hash_; }

        std::size_t GetLength() const
        { return length_; }

        // Convert to std::string
        std::string GetStdString() const;

        // Change context of string
        void SetValue(const std::string &str);
        void SetValue(const char *str);
        void SetValue(const char *str, std::size_t len);

        friend bool operator == (const String &l, const String &r)
        {
            return l.hash_ == r.hash_ &&
                l.length_ == r.length_ &&
                (l.in_heap_ ? memcmp(l.str_, r.str_, l.length_) :
                 memcmp(l.str_buffer_, r.str_buffer_, l.length_)) == 0;
        }

        friend bool operator != (const String &l, const String &r)
        {
            return !(l == r);
        }

        friend bool operator < (const String &l, const String &r)
        {
            auto *l_s = l.in_heap_ ? l.str_ : l.str_buffer_;
            auto *r_s = r.in_heap_ ? r.str_ : r.str_buffer_;
            auto len = std::min(l.length_, r.length_);
            auto cmp = memcmp(l_s, r_s, len);
            if (cmp == 0)
                return l.length_ < r.length_;
            else
                return cmp < 0;
        }

        friend bool operator >= (const String &l, const String &r)
        {
            return !(l < r);
        }

        friend bool operator > (const String &l, const String &r)
        {
            return r < l;
        }

        friend bool operator <= (const String &l, const String &r)
        {
            return !(l > r);
        }

    private:
        // Calculate hash of string
        void Hash(const char *s);

        // String in heap or not
        char in_heap_;
        union
        {
            // Buffer for short string
            char str_buffer_[11];
            // Pointer to heap which stored long string
            char *str_;
        };

        // Length of string
        unsigned int length_;
        // Hash value of string
        std::size_t hash_;
    };
} // namespace luna

#endif // STRING_H
