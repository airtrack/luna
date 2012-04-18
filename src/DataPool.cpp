#include "DataPool.h"

namespace lua
{
    DataPool::DataPool()
        : true_(true),
          false_(false),
          number_pool_(0),
          string_pool_(0),
          table_pool_(0),
          table_value_pool_(0),
          function_pool_(0),
          closure_pool_(0),
          native_func_pool_(0)
    {
    }

    DataPool::~DataPool()
    {
        ReleasePool(number_pool_);
        ReleasePool(string_pool_);
        ReleasePool(table_pool_);
        ReleasePool(table_value_pool_);
        ReleasePool(function_pool_);
        ReleasePool(closure_pool_);
        ReleasePool(native_func_pool_);
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

    Bool * DataPool::GetBool(bool b)
    {
        return b ? &true_ : &false_;
    }

    Number * DataPool::GetNumber(double number)
    {
        return NewElem(number_pool_, number);
    }

    String * DataPool::GetString(const std::string& str)
    {
        return NewElem(string_pool_, str);
    }

    Table * DataPool::GetTable()
    {
        return NewElem(table_pool_, this);
    }

    TableValue * DataPool::GetTableValue(Value *value)
    {
        return NewElem(table_value_pool_, value);
    }

    Function * DataPool::GetFunction(std::unique_ptr<UpvalueNameSet> &&upvalue_set)
    {
        FunctionPoolElement *elem =
            new FunctionPoolElement(std::move(upvalue_set), function_pool_);
        function_pool_ = elem;
        return &elem->elem;
    }

    Closure * DataPool::GetClosure(Function *func)
    {
        Table *upvalue_table = func->HasUpvalue() ? GetTable() : 0;
        ClosurePoolElement *elem =
            new ClosurePoolElement(func, upvalue_table, closure_pool_);
        closure_pool_ = elem;
        return &elem->elem;
    }

    NativeFunction * DataPool::GetNativeFunction(const NativeFunction::FuncType& func)
    {
        return NewElem(native_func_pool_, func);
    }
} // namespace lua
