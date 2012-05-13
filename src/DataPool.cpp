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
        on_alloc_();
        FunctionPoolElement *elem =
            new FunctionPoolElement(std::move(upvalue_set), function_pool_);
        function_pool_ = elem;
        return &elem->elem;
    }

    Closure * DataPool::GetClosure(Function *func)
    {
        on_alloc_();
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

    void DataPool::Sweep()
    {
        nil_.UnmarkSelf();
        true_.UnmarkSelf();
        false_.UnmarkSelf();

        SweepPool(number_pool_);
        SweepPool(string_pool_);
        SweepPool(table_pool_);
        SweepPool(table_value_pool_);
        SweepPool(function_pool_);
        SweepPool(closure_pool_);
        SweepPool(native_func_pool_);
    }

    void DataPool::SetOnAlloc(const StatFunc& on_alloc)
    {
        on_alloc_ = on_alloc;
    }

    void DataPool::SetOnDealloc(const StatFunc& on_dealloc)
    {
        on_dealloc_ = on_dealloc;
    }
} // namespace lua
