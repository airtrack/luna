#ifndef STRING_POOL_H
#define STRING_POOL_H

#include "String.h"
#include <vector>
#include <unordered_set>

namespace luna
{
    class StringPool
    {
    public:
        StringPool();
        ~StringPool();

        StringPool(const StringPool&) = delete;
        void operator = (const StringPool&) = delete;

        String * AllocString(const std::string &str);

    private:
        struct StringHash
        {
            std::size_t operator () (const String *s) const
            {
                return std::hash<std::string>()(s->GetStdString());
            }
        };

        struct StringEqual
        {
            bool operator () (const String *l, const String *r) const
            {
                return l->GetStdString() == r->GetStdString();
            }
        };

        String * GetSpareString();
        void UseSpareString(String *spare);

        // all used String store in it
        std::unordered_set<String *, StringHash, StringEqual> strings_;
        // all unused String for spare
        std::vector<String *> spares_;
    };
} // namespace luna

#endif // STRING_POOL_H
