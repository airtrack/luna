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
        return new Table(this);
    }

    TableValue * DataPool::GetTableValue(Value *value)
    {
        return new TableValue(value);
    }

    Function * DataPool::GetFunction(std::unique_ptr<NameSet> &&up_value_set)
    {
        return new Function(std::move(up_value_set));
    }

    Closure * DataPool::GetClosure(Function *func)
    {
        Table *upvalue_table = func->HasUpvalue() ? GetTable() : 0;
        return new Closure(func, upvalue_table);
    }

    NativeFunction * DataPool::GetNativeFunction(const NativeFunction::FuncType& func)
    {
        return new NativeFunction(func);
    }
} // namespace lua
