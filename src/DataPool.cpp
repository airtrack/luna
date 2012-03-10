#include "DataPool.h"

namespace lua
{
    DataPool::DataPool()
        : true_(true),
          false_(false)
    {
    }

    Nil * DataPool::GetNil()
    {
        return &nil_;
    }

    Bool * DataPool::GetTrue()
    {
        return &true_;
    }

    Bool * DataPool::GetFalse()
    {
        return &false_;
    }

    Number * DataPool::GetNumber(double number)
    {
        return new Number(number);
    }

    String * DataPool::GetString(const std::string& str)
    {
        return new String(str);
    }

    Table * DataPool::GetTable()
    {
        return new Table;
    }

    Function * DataPool::GetFunction()
    {
        return new Function;
    }
} // namespace lua
