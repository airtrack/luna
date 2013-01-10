#include "StringPool.h"
#include <assert.h>

namespace luna
{
    StringPool::StringPool()
    {
    }

    StringPool::~StringPool()
    {
        for (auto p : strings_)
            delete p;
        for (auto p : spares_)
            delete p;
    }

    String * StringPool::AllocString(const std::string &str)
    {
        String *spare = GetSpareString();
        spare->SetValue(str);

        auto it = strings_.find(spare);
        if (it == strings_.end())
        {
            // if the 'str' is not existed, insert into strings_
            strings_.insert(spare);
            UseSpareString(spare);
        }
        else
        {
            // if the 'str' value is existed, just use the existed one
            spare = *it;
        }

        return spare;
    }

    String * StringPool::GetSpareString()
    {
        if (spares_.empty())
            spares_.push_back(new String);
        return spares_.back();
    }

    void StringPool::UseSpareString(String *spare)
    {
        assert(!spares_.empty());
        assert(spares_.back() == spare);
        spares_.pop_back();
    }
} // namespace luna
