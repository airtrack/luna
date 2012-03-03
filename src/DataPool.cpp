#include "DataPool.h"

namespace lua
{
    Nil * DataPool::GetNil()
    {
        return 0;
    }

    Bool * DataPool::GetTrue()
    {
        return 0;
    }

    Bool * DataPool::GetFalse()
    {
        return 0;
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
        return 0;
    }

    Function * DataPool::GetFunction()
    {
        return 0;
    }
} // namespace lua
