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

    Function * DataPool::GetFunction(std::unique_ptr<NameSet>&& up_value_set)
    {
        return new Function(std::move(up_value_set));
    }

    NativeFunction * DataPool::GetNativeFunction(const NativeFunction::FuncType& func)
    {
        return new NativeFunction(func);
    }
} // namespace lua
