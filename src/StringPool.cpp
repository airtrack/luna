#include "StringPool.h"
#include <assert.h>

namespace luna
{
    StringPool::StringPool()
    {
    }

    String * StringPool::GetString(const std::string &str)
    {
        temp_.SetValue(str);
        auto it = strings_.find(&temp_);
        if (it == strings_.end())
            return nullptr;
        else
            return *it;
    }

    void StringPool::AddString(String *str)
    {
        auto it = strings_.insert(str);
        assert(it.second);
        (void)it;
    }

    void StringPool::DeleteString(String *str)
    {
        strings_.erase(str);
    }
} // namespace luna
